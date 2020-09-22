#pragma once

#include "aqueue.h"
#include "buf.h"
#include "utils.h"

typedef void (*work_fn)(void*);
struct thread_pool_work_item {
    void* arg;
    work_fn fn;
};

struct thread_pool_worker_arg {
    size_t id;
    struct thread_pool* thread_pool;
};

struct thread_pool {
    size_t threads_len;
    struct aqueue queue;
    pthread_t* threads;
    struct thread_pool_worker_arg* worker_args;
    size_t stopped;
    struct allocator* allocator;
};

void* thread_pool_worker(void* v_arg) {
    PG_ASSERT_NOT_EQ(v_arg, NULL, "%p");
    struct thread_pool_worker_arg* arg = v_arg;
    PG_ASSERT_NOT_EQ(arg->thread_pool, NULL, "%p");

    int stopped;
    while ((stopped = __atomic_load_n(&arg->thread_pool->stopped,
                                      __ATOMIC_ACQUIRE)) == 0) {
        struct thread_pool_work_item* item = NULL;
        if (aqueue_pop(&arg->thread_pool->queue, (void**)&item) == 0) {
            PG_ASSERT_NOT_EQ(item, NULL, "%p");
            PG_ASSERT_NOT_EQ(item->fn, NULL, "%p");

            item->fn(item->arg);
        } else {
            pg_nanosleep(10);
        }
    }

    return NULL;
}

int thread_pool_init(struct thread_pool* thread_pool, size_t len,
                     struct allocator* allocator) {
    PG_ASSERT_NOT_EQ(thread_pool, NULL, "%p");
    PG_ASSERT_NOT_EQ(allocator, NULL, "%p");

    thread_pool->allocator = allocator;
    thread_pool->threads = NULL;
    thread_pool->threads = buf_grow(thread_pool->threads, len);

    int ret;
    if ((ret = aqueue_init(&thread_pool->queue, allocator)) != 0) {
        return ret;
    }

    thread_pool->threads_len = len;

    thread_pool->worker_args = NULL;
    thread_pool->worker_args = buf_grow(thread_pool->worker_args, len);

    thread_pool->stopped = 0;

    return 0;
}

void thread_pool_start(struct thread_pool* thread_pool) {
    PG_ASSERT_NOT_EQ(thread_pool, NULL, "%p");
    PG_ASSERT_NOT_EQ(thread_pool->threads, NULL, "%p");
    PG_ASSERT_NOT_EQ(thread_pool->threads_len, (size_t)0, "%zu");

    for (size_t i = 0; i < thread_pool->threads_len; i++) {
        buf_push(thread_pool->worker_args, (struct thread_pool_worker_arg){
                                               .id = i,
                                               .thread_pool = thread_pool,
                                           });
        buf_push(thread_pool->threads, NULL);

        PG_ASSERT_EQ(
            pthread_create(&thread_pool->threads[i], NULL, thread_pool_worker,
                           &thread_pool->worker_args[i]),
            0, "%d");
    }
}

void thread_pool_join(struct thread_pool* thread_pool) {
    PG_ASSERT_NOT_EQ(thread_pool, NULL, "%p");

    for (size_t i = 0; i < thread_pool->threads_len; i++) {
        PG_ASSERT_EQ(pthread_join(thread_pool->threads[i], NULL), 0, "%d");
    }
}

void thread_pool_stop(struct thread_pool* thread_pool) {
    PG_ASSERT_NOT_EQ(thread_pool, NULL, "%p");
    PG_ASSERT_NOT_EQ(thread_pool->threads, NULL, "%p");
    PG_ASSERT_NOT_EQ(thread_pool->threads_len, (size_t)0, "%zu");

    __atomic_fetch_add(&thread_pool->stopped, 1, __ATOMIC_ACQUIRE);
    thread_pool_join(thread_pool);
}

void thread_pool_wait_until_finished(struct thread_pool* thread_pool) {
    PG_ASSERT_NOT_EQ(thread_pool, NULL, "%p");
    PG_ASSERT_NOT_EQ(thread_pool->threads, NULL, "%p");
    PG_ASSERT_NOT_EQ(thread_pool->threads_len, (size_t)0, "%zu");

    while (aqueue_len(&thread_pool->queue) != 0) {
        pg_nanosleep(10);
    }
    __atomic_fetch_add(&thread_pool->stopped, 1, __ATOMIC_ACQUIRE);
    thread_pool_join(thread_pool);
}

int thread_pool_push(struct thread_pool* thread_pool,
                     struct thread_pool_work_item* work) {
    PG_ASSERT_NOT_EQ(thread_pool, NULL, "%p");
    PG_ASSERT_NOT_EQ(thread_pool->threads, NULL, "%p");
    PG_ASSERT_NOT_EQ(thread_pool->threads_len, (size_t)0, "%zu");
    PG_ASSERT_NOT_EQ(work, NULL, "%p");

    return aqueue_push(&thread_pool->queue, work);
}

void thread_pool_deinit(struct thread_pool* thread_pool) {
    if (thread_pool == NULL) return;

    PG_ASSERT_NOT_EQ(thread_pool->threads, NULL, "%p");
    PG_ASSERT_NOT_EQ(thread_pool->threads_len, (size_t)0, "%zu");
    PG_ASSERT_NOT_EQ(thread_pool->allocator, NULL, "%p");

    for (size_t i = 0; i < thread_pool->threads_len; i++) {
        pthread_join(thread_pool->threads[i], NULL);
    }

    if (thread_pool->threads != NULL) buf_free(thread_pool->threads);
    aqueue_deinit(&thread_pool->queue);  // FIXME: was it init-ed?

    if (thread_pool->worker_args != NULL) buf_free(thread_pool->worker_args);
}
