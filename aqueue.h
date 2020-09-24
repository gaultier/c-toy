#pragma once

#include "utils.h"

#define AQUEUE_CAPACITY 1000

// Thread safe, lockless, zero-allocations queue.
// TODO: check memory consistency levels (e.g __ATOMIC_ACQUIRE).

struct aqueue_node {
    void* data;
    size_t version;
};

struct aqueue {
    struct aqueue_node* nodes;
    size_t front, rear;
};

struct aqueue aqueue_from_buffer(struct aqueue_node* buffer) {
    return (struct aqueue){.front = 0, .rear = 0, .nodes = buffer};
}

size_t aqueue_len(struct aqueue* queue) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");

    return __atomic_load_n(&queue->rear, __ATOMIC_ACQUIRE) -
           __atomic_load_n(&queue->front, __ATOMIC_ACQUIRE);
}

int aqueue_push(struct aqueue* queue, void* data) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");
    PG_ASSERT_NOT_EQ(data, NULL, "%p");

    size_t trial = 0;
    while (trial++ < 10) {
        size_t rear = __atomic_load_n(&queue->rear, __ATOMIC_ACQUIRE);
        struct aqueue_node* x = &queue->nodes[rear % AQUEUE_CAPACITY];

        if (rear != __atomic_load_n(&queue->rear, __ATOMIC_ACQUIRE))
            continue;  // outdated view

        if (rear ==
            __atomic_load_n(&queue->front, __ATOMIC_ACQUIRE) + AQUEUE_CAPACITY)
            continue;  // full queue

        if (x->data == NULL) {  // free slot
            struct aqueue_node new_x = {.data = data,
                                        .version = x->version + 1};

            if (__atomic_compare_exchange(&queue->nodes[rear % AQUEUE_CAPACITY],
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

    size_t trials = 0;
    while (trials++ < 10) {
        size_t front = __atomic_load_n(&queue->front, __ATOMIC_ACQUIRE);
        struct aqueue_node x = queue->nodes[front % AQUEUE_CAPACITY];

        if (front != __atomic_load_n(&queue->front, __ATOMIC_ACQUIRE)) {
            continue;  // outdated view
        }

        if (front == __atomic_load_n(&queue->rear, __ATOMIC_ACQUIRE)) {
            continue;  // empty queue
        }

        if (x.data != NULL) {  // filled slot
            struct aqueue_node new_x = {.data = NULL, .version = x.version + 1};

            if (__atomic_compare_exchange(
                    &queue->nodes[front % AQUEUE_CAPACITY], &x, &new_x, 1,
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
