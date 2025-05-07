#ifndef HASHMAP_H
#define HASHMAP_H

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    int key;
    int value;
} item;

/*  Internal bucket node  */
typedef struct bucket_node {
    item                  data;
    struct bucket_node   *next;
} bucket_node;

/*  Hash‑table handle  */
typedef struct {
    bucket_node        **buckets;   /* array of heads            */
    pthread_mutex_t     *locks;     /* one lock per bucket       */
    int                  size;      /* number of buckets         */
} hashmap_t;

/*  API  */
hashmap_t *hm_init    (int size);
void       hm_put     (hashmap_t *hm, item kv);
bool       hm_get     (hashmap_t *hm, int key, item *out); /* returns true if found */
void       hm_destroy (hashmap_t *hm);                     /* clean up */

#endif
