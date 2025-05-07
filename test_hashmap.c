#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include "hashmap.h"

/* 100M per thread by default; shrink with –DOPERATIONS=n */
#ifndef OPERATIONS
#define OPERATIONS 100000000UL
#endif

#define PRINT_EVERY 5000000UL   /* progress granularity */

typedef struct {
    hashmap_t   *hm;
    unsigned long ops_done;
    int           id;
} thread_arg;

/* tiny think‑time helps highlight lock contention realistically */
static inline void tiny_pause(void) {
    /* 2‑3 CPU cycles on x86; fine‑grain back‑off */
    asm volatile("pause" ::: "memory");
}

static void *worker(void *arg_)
{
    thread_arg *arg = arg_;
    unsigned long ctr = 0;
    unsigned int  seed = (unsigned)time(NULL) ^ (unsigned)(uintptr_t)pthread_self();
    item          dummy;

    while (ctr < OPERATIONS) {
        int key  = rand_r(&seed) % 1000000;
        if (rand_r(&seed) & 1) {                     /* 50 % put */
            item kv = { key, rand_r(&seed) };
            hm_put(arg->hm, kv);
        } else {                                    /* 50 % get */
            hm_get(arg->hm, key, &dummy);
        }
        ++ctr;
        if ((ctr % PRINT_EVERY) == 0)
            fprintf(stderr, "[T%d] %lu ops\n", arg->id, ctr);

        tiny_pause();
    }
    arg->ops_done = ctr;
    return NULL;
}

static double bench(int threads, int buckets)
{
    hashmap_t   *hm = hm_init(buckets);
    pthread_t    tids[threads];
    thread_arg   args[threads];

    struct timespec s, e;
    clock_gettime(CLOCK_MONOTONIC, &s);

    for (int i = 0; i < threads; ++i) {
        args[i] = (thread_arg){ hm, 0, i };
        pthread_create(&tids[i], NULL, worker, &args[i]);
    }
    for (int i = 0; i < threads; ++i)
        pthread_join(tids[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &e);
    hm_destroy(hm);
    return (e.tv_sec - s.tv_sec) + (e.tv_nsec - s.tv_nsec) / 1e9;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <threads> <buckets>\n", argv[0]);
        return 1;
    }
    int threads = atoi(argv[1]);
    int buckets = atoi(argv[2]);

    fprintf(stderr, "Starting: %d thread(s), %d buckets, "
                    "%lu ops/thread (%.1f M total)\n",
            threads, buckets, (unsigned long)OPERATIONS,
            threads * OPERATIONS / 1e6);

    double secs = bench(threads, buckets);
    double perM = secs * 1e6 / (threads * OPERATIONS);

    printf("%d threads, %d buckets: %.2f s total, %.3f s per 1 M ops\n",
           threads, buckets, secs, perM);
    return 0;
}
