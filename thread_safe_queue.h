#pragma once
#include <errno.h>
#include <pthread.h>

#include "allocator.h"
#include "utils.h"

typedef void* thread_safe_queue_data_t;
struct thread_safe_queue {
    size_t capacity;
    size_t len;
    size_t start_current;
    thread_safe_queue_data_t* data;
    pthread_mutex_t mutex;
    struct allocator* allocator;
};

size_t thread_safe_queue_len(struct thread_safe_queue* queue) {
    return __atomic_load_n(&queue->len, __ATOMIC_ACQUIRE);
}

int thread_safe_queue_init(struct thread_safe_queue* queue,
                           struct allocator* allocator) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");
    PG_ASSERT_NOT_EQ(allocator, NULL, "%p");

    queue->allocator = allocator;
    queue->capacity = 1000;
    queue->data = allocator->realloc(
        NULL, queue->capacity * sizeof(thread_safe_queue_data_t));
    if (queue->data == NULL) return ENOMEM;

    queue->len = 0;
    queue->start_current = 0;

    int err;
    if ((err = pthread_mutex_init(&queue->mutex, NULL)) != 0) return err;

    return 0;
}

void thread_safe_queue_deinit(struct thread_safe_queue* queue) {
    if (queue == NULL) return;
    PG_ASSERT_NOT_EQ(queue->allocator, NULL, "%p");

    if (queue->data != NULL) queue->allocator->free(queue->data);

    pthread_mutex_destroy(&queue->mutex);  // FIXME: was `mutex` init-ed ?
}

int thread_safe_queue_push(struct thread_safe_queue* queue,
                           const thread_safe_queue_data_t item) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");
    PG_ASSERT_NOT_EQ(queue->data, NULL, "%p");
    PG_ASSERT_NOT_EQ(item, NULL, "%p");

    int ret;
    while ((ret = pthread_mutex_trylock(&queue->mutex)) == EBUSY) {
    }
    if (ret != 0) return ret;

    {
        PG_ASSERT_COND(thread_safe_queue_len(queue), <=, queue->capacity,
                       "%zu");
        PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu");

        if (thread_safe_queue_len(queue) == queue->capacity) {
            pthread_mutex_unlock(&queue->mutex);
            return ENOMEM;
        }
        const size_t i = (queue->start_current + thread_safe_queue_len(queue)) %
                         queue->capacity;
        buf_set_at(queue->data, queue->capacity, item, i);
        __atomic_fetch_add(&queue->len, 1, __ATOMIC_ACQUIRE);
    }

    PG_ASSERT_COND(thread_safe_queue_len(queue), <=, queue->capacity, "%zu");
    PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu");

    pthread_mutex_unlock(&queue->mutex);

    return 0;
}

int thread_safe_queue_pop(struct thread_safe_queue* queue,
                          thread_safe_queue_data_t* item) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");
    PG_ASSERT_NOT_EQ(queue->data, NULL, "%p");
    PG_ASSERT_NOT_EQ(item, NULL, "%p");

    int ret;
    while ((ret = pthread_mutex_trylock(&queue->mutex)) == EBUSY) {
    }
    if (ret != 0) return ret;

    {
        PG_ASSERT_COND(thread_safe_queue_len(queue), <=, queue->capacity,
                       "%zu");
        PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu");

        if (thread_safe_queue_len(queue) == 0) {
            pthread_mutex_unlock(&queue->mutex);
            return EINVAL;
        }

        buf_get_at(queue->data, queue->capacity, *item, queue->start_current);

        __atomic_fetch_add(&queue->len, -1, __ATOMIC_ACQUIRE);
        queue->start_current = (queue->start_current + 1) % queue->capacity;
    }

    PG_ASSERT_COND(thread_safe_queue_len(queue), <=, queue->capacity, "%zu");
    PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu");

    pthread_mutex_unlock(&queue->mutex);

    return 0;
}

