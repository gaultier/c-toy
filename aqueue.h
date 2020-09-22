#pragma once
#include <errno.h>
#include <pthread.h>

#include "allocator.h"
#include "utils.h"

typedef void* aqueue_data_t;
struct aqueue {
    size_t capacity;
    size_t len;
    size_t start_current;
    aqueue_data_t* data;
    pthread_mutex_t mutex;
    struct allocator* allocator;
};

size_t aqueue_len(struct aqueue* queue) {
    return __atomic_load_n(&queue->len, __ATOMIC_ACQUIRE);
}

int aqueue_init(struct aqueue* queue,
                           struct allocator* allocator) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");
    PG_ASSERT_NOT_EQ(allocator, NULL, "%p");

    queue->allocator = allocator;
    queue->capacity = 1000;
    queue->data = allocator->realloc(
        NULL, queue->capacity * sizeof(aqueue_data_t));
    if (queue->data == NULL) return ENOMEM;

    queue->len = 0;
    queue->start_current = 0;

    int err;
    if ((err = pthread_mutex_init(&queue->mutex, NULL)) != 0) return err;

    return 0;
}

void aqueue_deinit(struct aqueue* queue) {
    if (queue == NULL) return;
    PG_ASSERT_NOT_EQ(queue->allocator, NULL, "%p");

    if (queue->data != NULL) queue->allocator->free(queue->data);

    pthread_mutex_destroy(&queue->mutex);  // FIXME: was `mutex` init-ed ?
}

int aqueue_push(struct aqueue* queue,
                           const aqueue_data_t item) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");
    PG_ASSERT_NOT_EQ(queue->data, NULL, "%p");
    PG_ASSERT_NOT_EQ(item, NULL, "%p");

    int ret;
    while ((ret = pthread_mutex_trylock(&queue->mutex)) == EBUSY) {
    }
    if (ret != 0) return ret;

    {
        PG_ASSERT_COND(aqueue_len(queue), <=, queue->capacity,
                       "%zu");
        PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu");

        if (aqueue_len(queue) == queue->capacity) {
            pthread_mutex_unlock(&queue->mutex);
            return ENOMEM;
        }
        const size_t i = (queue->start_current + aqueue_len(queue)) %
                         queue->capacity;
        buf_set_at(queue->data, queue->capacity, item, i);
        __atomic_fetch_add(&queue->len, 1, __ATOMIC_ACQUIRE);
    }

    PG_ASSERT_COND(aqueue_len(queue), <=, queue->capacity, "%zu");
    PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu");

    pthread_mutex_unlock(&queue->mutex);

    return 0;
}

int aqueue_pop(struct aqueue* queue,
                          aqueue_data_t* item) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");
    PG_ASSERT_NOT_EQ(queue->data, NULL, "%p");
    PG_ASSERT_NOT_EQ(item, NULL, "%p");

    int ret;
    while ((ret = pthread_mutex_trylock(&queue->mutex)) == EBUSY) {
    }
    if (ret != 0) return ret;

    {
        PG_ASSERT_COND(aqueue_len(queue), <=, queue->capacity,
                       "%zu");
        PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu");

        if (aqueue_len(queue) == 0) {
            pthread_mutex_unlock(&queue->mutex);
            return EINVAL;
        }

        buf_get_at(queue->data, queue->capacity, *item, queue->start_current);

        __atomic_fetch_add(&queue->len, -1, __ATOMIC_ACQUIRE);
        queue->start_current = (queue->start_current + 1) % queue->capacity;
    }

    PG_ASSERT_COND(aqueue_len(queue), <=, queue->capacity, "%zu");
    PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu");

    pthread_mutex_unlock(&queue->mutex);

    return 0;
}

