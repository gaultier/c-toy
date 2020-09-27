#pragma once

#include "utils.h"

// Thread safe, lockless, zero-allocations queue.

struct aqueue_node {
    void* data;
    size_t version;
};

struct aqueue {
    struct aqueue_node* nodes;
    size_t capacity;
    size_t front, rear;
};

void aqueue_init(struct aqueue* queue, struct aqueue_node* nodes,
                 size_t capacity) {
    queue->front = 0;
    queue->rear = 0;
    queue->capacity = capacity;
    queue->nodes = nodes;
    memset(queue->nodes, 0, sizeof(struct aqueue_node) * capacity);
}

size_t aqueue_len(struct aqueue* queue) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");

    return __atomic_load_n(&queue->rear, __ATOMIC_ACQUIRE) -
           __atomic_load_n(&queue->front, __ATOMIC_ACQUIRE);
}

int aqueue_push(struct aqueue* queue, void* data) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");
    PG_ASSERT_NOT_EQ(queue->nodes, NULL, "%p");
    PG_ASSERT_NOT_EQ(data, NULL, "%p");

    size_t trial = 0;
    while (trial++ < 10) {
        size_t rear = __atomic_load_n(&queue->rear, __ATOMIC_ACQUIRE);
        struct aqueue_node* x = &queue->nodes[rear % queue->capacity];

        if (rear != __atomic_load_n(&queue->rear, __ATOMIC_ACQUIRE))
            continue;  // outdated view

        if (rear ==
            __atomic_load_n(&queue->front, __ATOMIC_ACQUIRE) + queue->capacity)
            continue;  // full queue

        if (x->data == NULL) {  // free slot
            struct aqueue_node new_x = {.data = data,
                                        .version = x->version + 1};

            if (__atomic_compare_exchange(&queue->nodes[rear % queue->capacity],
                                          x, &new_x, 1, __ATOMIC_RELEASE,
                                          __ATOMIC_RELEASE)) {
                size_t new_rear = rear + 1;

                __atomic_compare_exchange(
                    &queue->rear, &rear, &new_rear, 1, __ATOMIC_RELEASE,
                    __ATOMIC_RELEASE);  // try to increment rear

                return 0;
            }
        } else {
            size_t new_rear = rear + 1;

            __atomic_compare_exchange(
                &queue->rear, &rear, &new_rear, 1, __ATOMIC_RELEASE,
                __ATOMIC_RELEASE);  // help others increment rear
        }
    }
    return EBUSY;
}

void* aqueue_pop(struct aqueue* queue) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");
    PG_ASSERT_NOT_EQ(queue->nodes, NULL, "%p");

    size_t trials = 0;
    while (trials++ < 10) {
        size_t front = __atomic_load_n(&queue->front, __ATOMIC_ACQUIRE);

        struct aqueue_node x = queue->nodes[front % queue->capacity];

        if (front != __atomic_load_n(&queue->front, __ATOMIC_ACQUIRE)) {
            continue;  // outdated view
        }

        if (front == __atomic_load_n(&queue->rear, __ATOMIC_ACQUIRE)) {
            continue;  // empty queue
        }

        if (x.data != NULL) {  // filled slot
            struct aqueue_node new_x = {.data = NULL, .version = x.version + 1};

            if (__atomic_compare_exchange(
                    &queue->nodes[front % queue->capacity], &x, &new_x, 1,
                    __ATOMIC_RELEASE, __ATOMIC_RELEASE)) {
                size_t new_front = front + 1;

                __atomic_compare_exchange(
                    &queue->front, &front, &new_front, 1, __ATOMIC_RELEASE,
                    __ATOMIC_RELEASE);  // try to increment front

                return x.data;
            }
        } else {
            size_t new_front = front + 1;

            __atomic_compare_exchange(
                &queue->front, &front, &new_front, 1, __ATOMIC_RELEASE,
                __ATOMIC_RELEASE);  // help others increment front
        }
    }
    return NULL;
}
