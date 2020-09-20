#pragma once
#include <time.h>

#include "thread_safe_queue.h"
#include "utils.h"

typedef void (*work_fn)(void*);
struct work_item {
    void* arg;
    work_fn fn;
};

struct worker_arg {
    size_t id;
    struct thread_pool* thread_pool;
};

struct thread_pool {
    size_t threads_len;
    struct thread_safe_queue queue;
    pthread_t* threads;
    struct worker_arg* worker_args;
    size_t stopped;
};

void* worker(void* v_arg) {
    struct worker_arg* arg = v_arg;

    int stopped;
    while ((stopped = __atomic_load_n(&arg->thread_pool->stopped,
                                      __ATOMIC_ACQUIRE)) == 0) {
        struct work_item* item = NULL;
        if (thread_safe_queue_pop(&arg->thread_pool->queue, (void**)&item) ==
            0) {
            PG_ASSERT_NOT_EQ(item, NULL, "%p");
            PG_ASSERT_NOT_EQ(item->fn, NULL, "%p");

            item->fn(item->arg);
        } else {
            const struct timespec time = {.tv_nsec = 10};
            nanosleep(&time, NULL);
        }
    }

    return NULL;
}

int thread_pool_init(struct thread_pool* thread_pool, size_t len,
                     struct allocator* allocator) {
    thread_pool->threads = allocator->realloc(NULL, len * sizeof(pthread_t));
    if (thread_pool->threads == NULL) return ENOMEM;

    int ret;
    if ((ret = thread_safe_queue_init(&thread_pool->queue, allocator)) != 0) {
        thread_safe_queue_deinit(&thread_pool->queue, allocator);
        return ret;
    }

    thread_pool->threads_len = len;

    thread_pool->worker_args =
        allocator->realloc(NULL, len * sizeof(struct worker_arg));
    if (thread_pool->worker_args == NULL) return ENOMEM;

    thread_pool->stopped = 0;

    return 0;
}

void thread_pool_start(struct thread_pool* thread_pool) {
    for (size_t i = 0; i < thread_pool->threads_len; i++) {
        thread_pool->worker_args[i] = (struct worker_arg){
            .id = i,
            .thread_pool = thread_pool,
        };
        PG_ASSERT_EQ(pthread_create(&thread_pool->threads[i], NULL, worker,
                                    &thread_pool->worker_args[i]),
                     0, "%d");
    }
}

void thread_pool_join(struct thread_pool* thread_pool) {
    for (size_t i = 0; i < thread_pool->threads_len; i++) {
        PG_ASSERT_EQ(pthread_join(thread_pool->threads[i], NULL), 0, "%d");
    }
}

void thread_pool_stop(struct thread_pool* thread_pool) {
    __atomic_fetch_add(&thread_pool->stopped, 1, __ATOMIC_ACQUIRE);
    thread_pool_join(thread_pool);
}

int thread_pool_push(struct thread_pool* thread_pool, struct work_item* work) {
    return thread_safe_queue_push(&thread_pool->queue, work);
}

void thread_pool_deinit(struct thread_pool* thread_pool,
                        struct allocator* allocator) {
    for (size_t i = 0; i < thread_pool->threads_len; i++) {
        pthread_join(thread_pool->threads[i], NULL);
    }

    if (thread_pool->threads != NULL) allocator->free(thread_pool->threads);
    thread_safe_queue_deinit(&thread_pool->queue,
                             allocator);  // FIXME: was it init-ed?

    if (thread_pool->worker_args != NULL)
        allocator->free(thread_pool->worker_args);
}

int thread_pool_work_push(struct thread_pool* thread_pool,
                          struct work_item* work) {
    return thread_safe_queue_push(&thread_pool->queue, work);
}