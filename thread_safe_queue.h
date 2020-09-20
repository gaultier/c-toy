#pragma once
#include <errno.h>
#include <pthread.h>

#include "allocator.h"

typedef void* data_t;
struct thread_safe_queue {
    size_t capacity;
    size_t len;
    size_t start_current;
    data_t* data;
    pthread_mutex_t mutex;
};

int thread_safe_queue_init(struct thread_safe_queue* queue,
                           struct allocator* allocator) {
    queue->capacity = 1000;
    queue->data = allocator->realloc(NULL, queue->capacity * sizeof(data_t));
    if (queue->data == NULL) return ENOMEM;

    queue->len = 0;
    queue->start_current = 0;

    int err;
    if ((err = pthread_mutex_init(&queue->mutex, NULL)) != 0) return err;

    return 0;
}

void thread_safe_queue_deinit(struct thread_safe_queue* queue,
                              struct allocator* allocator) {
    if (queue->data != NULL) allocator->free(queue->data);

    pthread_mutex_destroy(&queue->mutex);  // FIXME: was `mutex` init-ed ?
}

int thread_safe_queue_push(struct thread_safe_queue* queue, data_t item) {
    int err;
    if ((err = pthread_mutex_lock(&queue->mutex)) != 0) return err;

    {
        if (queue->len == queue->capacity) return ENOMEM;
        const size_t i = (queue->start_current + queue->len) % queue->capacity;
        queue->data[i] = item;
        queue->len += 1;
    }

    if ((err = pthread_mutex_unlock(&queue->mutex)) != 0) return err;

    return 0;
}

int thread_safe_queue_pop(struct thread_safe_queue* queue, data_t* item) {
    int err;
    if ((err = pthread_mutex_lock(&queue->mutex)) != 0) return err;

    {
        if (queue->len == 0) return EINVAL;

        *item = queue->data[queue->start_current];
        queue->len -= 1;
        queue->start_current += 1;
    }

    if ((err = pthread_mutex_unlock(&queue->mutex)) != 0) return err;

    return 0;
}
