#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"

/* ------------------------------------------------  init / destroy */
hashmap_t *hm_init(int size)
{
    hashmap_t *hm = calloc(1, sizeof *hm);
    hm->size    = size;
    hm->buckets = calloc(size, sizeof *hm->buckets);
    hm->locks   = calloc(size, sizeof *hm->locks);
    for (int i = 0; i < size; ++i)
        pthread_mutex_init(&hm->locks[i], NULL);
    return hm;
}

void hm_destroy(hashmap_t *hm)
{
    for (int i = 0; i < hm->size; ++i) {
        bucket_node *cur = hm->buckets[i];
        while (cur) {
            bucket_node *tmp = cur->next;
            free(cur);
            cur = tmp;
        }
        pthread_mutex_destroy(&hm->locks[i]);
    }
    free(hm->buckets);
    free(hm->locks);
    free(hm);
}

/* ---------------------------------------------------------  put() */
void hm_put(hashmap_t *hm, item kv)
{
    int idx = kv.key % hm->size;
    pthread_mutex_lock(&hm->locks[idx]);

    bucket_node *cur = hm->buckets[idx];
    while (cur) {                         /* update if key exists */
        if (cur->data.key == kv.key) {
            cur->data.value = kv.value;
            pthread_mutex_unlock(&hm->locks[idx]);
            return;
        }
        cur = cur->next;
    }
    /* insert at head */
    bucket_node *node = malloc(sizeof *node);
    node->data = kv;
    node->next = hm->buckets[idx];
    hm->buckets[idx] = node;

    pthread_mutex_unlock(&hm->locks[idx]);
}

/* ----------------------------------------------------------  get() */
bool hm_get(hashmap_t *hm, int key, item *out)
{
    int idx = key % hm->size;
    pthread_mutex_lock(&hm->locks[idx]);

    bucket_node *cur = hm->buckets[idx];
    while (cur) {
        if (cur->data.key == key) {
            *out = cur->data;
            pthread_mutex_unlock(&hm->locks[idx]);
            return true;
        }
        cur = cur->next;
    }
    pthread_mutex_unlock(&hm->locks[idx]);
    return false;
}
