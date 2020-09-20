#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/* #include "aco.h" */

typedef void* work_item;
struct thread_pool {
    size_t len;
    work_item* work_queue;
    size_t work_queue_capacity;
    size_t work_queue_len;
    size_t work_queue_start_current;
    pthread_t* threads;
    pthread_mutex_t* work_queue_mutex;
};

typedef void (*work_fn)(work_item);
void worker(work_item* work_queue, work_fn fn) {
    while (1) {
    }
}

typedef void (*free_fn)(void*);
typedef void* (*realloc_fn)(void*, size_t);
struct allocator {
    free_fn free;
    realloc_fn realloc;
};

int thread_pool_init(size_t len, struct thread_pool* thread_pool,
                     struct allocator* allocator) {
    thread_pool->threads = allocator->realloc(NULL, len * sizeof(pthread_t));
    if (thread_pool->threads == NULL) return ENOMEM;

    const size_t work_queue_capacity = 1000;
    thread_pool->work_queue =
        allocator->realloc(NULL, work_queue_capacity * sizeof(work_item));
    if (thread_pool->work_queue == NULL) return ENOMEM;

    thread_pool->len = len;
    thread_pool->work_queue_capacity = work_queue_capacity * sizeof(work_item);
    thread_pool->work_queue_len = 0;

    int err;
    if ((err = pthread_mutex_init(thread_pool->work_queue_mutex, NULL)) != 0)
        return err;

    return 0;
}

void thread_pool_deinit(struct thread_pool* thread_pool,
                        struct allocator* allocator) {
    if (thread_pool->threads != NULL) allocator->free(thread_pool->threads);
    if (thread_pool->work_queue != NULL)
        allocator->free(thread_pool->work_queue);
}

void thread_pool_work_pop(struct thread_pool* thread_pool) {}

int thread_pool_work_push(struct thread_pool* thread_pool, work_item* work) {
    int err;
    if ((err = pthread_mutex_lock(thread_pool->work_queue_mutex)) != 0)
        return err;

    // FIXME
    /* thread_pool->work_queue_current_start_current = */
    /*     (thread_pool->work_queue_start_current + 1) % */
    /*     thread_pool->work_queue_capacity; */
    /* thread_pool->[thread_pool->work_queue_current] = work; */

    if ((err = pthread_mutex_unlock(thread_pool->work_queue_mutex)) != 0)
        return err;

    return 0;
}

/* void ping() { */
/* int count = 0; */
/* while (count++ < 5) { */
/*     puts("ping"); */
/*     aco_yield(); */
/* } */
/* aco_exit(); */
/* } */

/* void pong() { */
/* int count = 0; */
/* while (count++ < 5) { */
/*     puts("pong"); */
/*     aco_yield(); */
/* } */
/* aco_exit(); */
/* } */

int main() {
    /* aco_thread_init(NULL); */

    /* aco_t* main_co = aco_create(NULL, NULL, 0, NULL, NULL); */

    /* aco_share_stack_t* sstk = aco_share_stack_new(0); */
    /* int ping_co_ct_arg_point_to_me = 0; */
    /* aco_t* ping_co = */
    /*     aco_create(main_co, sstk, 0, ping, &ping_co_ct_arg_point_to_me); */

    /* int pong_co_ct_arg_point_to_me = 0; */
    /* aco_t* pong_co = */
    /*     aco_create(main_co, sstk, 0, pong, &pong_co_ct_arg_point_to_me); */

    /* while (!ping_co->is_end && !pong_co->is_end) { */
    /*     aco_resume(ping_co); */
    /*     aco_resume(pong_co); */
    /* } */

    /* puts("The End"); */

    /* aco_destroy(pong_co); */
    /* pong_co = NULL; */
    /* aco_destroy(ping_co); */
    /* ping_co = NULL; */
    /* // destroy the share stack sstk. */
    /* aco_share_stack_destroy(sstk); */
    /* sstk = NULL; */
    /* // destroy the main_co. */
    /* aco_destroy(main_co); */
    /* main_co = NULL; */

    struct allocator allocator = {.realloc = realloc, .free = free};
    struct thread_pool thread_pool;
    int err = 0;
    if ((err = thread_pool_init(4, &thread_pool, &allocator)) != 0) return err;

    thread_pool_deinit(&thread_pool, &allocator);

    return 0;
}
