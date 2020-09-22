#pragma once

#include "utils.h"

struct aqueue_node {
    void* data;
    size_t version;
};

#define AQUEUE_CAPACITY 1000

struct aqueue {
    struct aqueue_node nodes[AQUEUE_CAPACITY];
    _Atomic size_t front, rear;
};

/* size_t aqueue_len(struct aqueue* queue) { */
/*     return __atomic_load_n(&queue->len, __ATOMIC_ACQUIRE); */
/* } */

void aqueue_push(struct aqueue* queue, struct aqueue_node* node) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");
    PG_ASSERT_NOT_EQ(node, (struct aqueue_node*)NULL, "%p");

    while (1) {
        size_t rear = queue->rear;
        struct aqueue_node* x = &queue->nodes[rear % AQUEUE_CAPACITY];
        if (rear != queue->rear) continue;                     // outdated view
        if (rear == queue->front + AQUEUE_CAPACITY) continue;  // full queue
        if (x->data == NULL) {                                 // free slot
            struct aqueue_node new_x = {.data = x->data,
                                        .version = x->version + 1};
            if (__atomic_compare_exchange(&queue->nodes[rear % AQUEUE_CAPACITY],
                                          x, &new_x, 1, __ATOMIC_ACQUIRE,
                                          __ATOMIC_ACQUIRE)) {
                size_t new_rear = rear + 1;
                __atomic_compare_exchange(
                    &queue->rear, rear, &new_rear, 1, __ATOMIC_ACQUIRE,
                    __ATOMIC_ACQUIRE);  // try to increment rear
                return;
            }
        } else {
            size_t new_rear = rear + 1;
            __atomic_compare_exchange(
                &queue->rear, rear, &new_rear, 1, __ATOMIC_ACQUIRE,
                __ATOMIC_ACQUIRE);  // help others increment rear
        }
    }
}
