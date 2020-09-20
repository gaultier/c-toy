#pragma once
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "allocator.h"

typedef void* data_t;
struct thread_safe_queue {
    size_t capacity;
    size_t len;
    size_t start_current;
    data_t* data;
    pthread_mutex_t mutex;
};

#define buf_get_at(data, len, val, index)                                   \
    do {                                                                    \
        if (index >= len) {                                                 \
            fprintf(stderr,                                                 \
                    "Error: accessing array at index %zu but len is %zu\n", \
                    index, len);                                            \
            exit(EINVAL);                                                   \
        }                                                                   \
        val = data[index];                                                  \
    } while (0)

void buf_get_at_ptr(data_t* data, size_t len, data_t* val, size_t index) {
    if (index >= len) {
        fprintf(stderr, "Error: accessing array at index %zu but len is %zu\n",
                index, len);
        exit(EINVAL);
    }

    *val = &(data[index]);
}

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

int thread_safe_queue_push(struct thread_safe_queue* queue, const data_t item) {
    int err;
    if ((err = pthread_mutex_lock(&queue->mutex)) != 0) return err;

    {
        if (queue->len == queue->capacity) {
            pthread_mutex_unlock(&queue->mutex);
            return ENOMEM;
        }
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
        if (queue->len == 0) {
            pthread_mutex_unlock(&queue->mutex);
            return EINVAL;
        }

        buf_get_at(queue->data, queue->capacity, *item, queue->start_current);
        queue->len -= 1;
        queue->start_current = (queue->start_current + 1) % queue->capacity;
    }

    if ((err = pthread_mutex_unlock(&queue->mutex)) != 0) return err;

    return 0;
}

