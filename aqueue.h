#pragma once

#include "buf.h"
#include "utils.h"

typedef void* aqueue_data_t;

struct aqueue_node;
struct aqueue_node {
    void* data;
    struct aqueue_node* next;
};

struct aqueue_node_head {
    size_t aba;
    struct aqueue_node* node;
};

struct aqueue {
    size_t len;
    struct aqueue_node_head head;
    /* struct aqueue_node_head* tail; */
    struct aqueue_node_head free;
    struct aqueue_node* buffer;
};

size_t aqueue_len(struct aqueue* queue) {
    return __atomic_load_n(&queue->len, __ATOMIC_ACQUIRE);
}

int aqueue_init(struct aqueue* queue, size_t max_size) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");

    queue->len = 0;
    queue->buffer = NULL;

    buf_grow(queue->buffer, max_size);

    for (size_t i = 0; i < max_size; i++) {
        struct aqueue_node node = {.data = NULL, .next = &queue->buffer[i + 1]};
        buf_push(queue->buffer, node);
    }

    queue->head.aba = 0;
    queue->head.node = NULL;

    queue->free.aba = 0;
    queue->free.node = queue->buffer;

    /* queue->tail = NULL; */

    return 0;
}

void aqueue_deinit(struct aqueue* queue) {
    if (queue == NULL) return;

    buf_free(queue->buffer);
}

struct aqueue_node* aqueue_pop1(struct aqueue_node_head* head) {
    struct aqueue_node_head next;
    struct aqueue_node_head* original_head = NULL;

    __atomic_load(head, original_head, __ATOMIC_SEQ_CST);

    do {
        if (original_head->node == NULL) return NULL;

        next.aba = original_head->aba + 1;
        next.node = original_head->node->next;
    } while (!__atomic_compare_exchange(head, original_head, &next, 1,
                                        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
    return original_head->node;
}

void aqueue_push1(struct aqueue_node_head* head, struct aqueue_node* node) {
    struct aqueue_node_head next;
    struct aqueue_node_head* original_head = NULL;

    __atomic_load(head, original_head, __ATOMIC_SEQ_CST);

    do {
        node->next = original_head->node;
        next.aba = original_head->aba + 1;
        next.node = node;
    } while (!__atomic_compare_exchange(head, original_head, &next, 1,
                                        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
}

void* aqueue_pop(aqueue* queue) {
    struct aqueue_node* node = aqueue_pop1(&queue->head);
    if (node == NULL) return NULL;

    __atomic_fetch_sub(&queue->len, 1, __ATOMIC_SEQ_CST);
    void* value = node->value;
    aqueue_push1(&queue->free, node);

    return value;
}

int aqueue_push(aqueue* queue, void* data) {
    struct aqueue_node* node = aqueue_pop1(queue->free);
    if (node == NULL) return ENOMEM;

    node->data = data;
    aqueue_push1(&queue->head, node);

    __atomic_fetch_add(&queue->len, 1, __ATOMIC_SEQ_CST);

    return 0;
}
