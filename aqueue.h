#pragma once

#include "buf.h"
#include "utils.h"

typedef void* aqueue_data_t;

struct aqueue_node;
struct aqueue_node {
    void* data;
    struct aqueue_node* next;
};

struct aqueue {
    size_t len;
    struct aqueue_node* head;
    struct aqueue_node* tail;
    struct aqueue_node* free;
    struct aqueue_node* buffer;
};

size_t aqueue_len(struct aqueue* queue) {
    return __atomic_load_n(&queue->len, __ATOMIC_ACQUIRE);
}

int aqueue_init(struct aqueue* queue) {
    PG_ASSERT_NOT_EQ(queue, NULL, "%p");

    queue->len = 0;
    queue->buffer = NULL;

    buf_grow(queue->buffer, 1000);

    for (size_t i = 0; i < 1000; i++) {
        struct aqueue_node node = {.data = NULL, .next = &queue->buffer[i + 1]};
        buf_push(queue->buffer, node);
    }

    queue->head = NULL;
    queue->tail = NULL;

    return 0;
}

void aqueue_deinit(struct aqueue* queue) {
    if (queue == NULL) return;

    buf_free(queue->buffer);
}

/* int aqueue_push(struct aqueue* queue, const aqueue_data_t item) { */
/*     PG_ASSERT_NOT_EQ(queue, NULL, "%p"); */
/*     PG_ASSERT_NOT_EQ(queue->data, NULL, "%p"); */
/*     PG_ASSERT_NOT_EQ(item, NULL, "%p"); */

/*     int ret; */
/*     while ((ret = pthread_mutex_trylock(&queue->mutex)) == EBUSY) { */
/*     } */
/*     if (ret != 0) return ret; */

/*     { */
/*         PG_ASSERT_COND(aqueue_len(queue), <=, queue->capacity, "%zu"); */
/*         PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu"); */

/*         if (aqueue_len(queue) == queue->capacity) { */
/*             pthread_mutex_unlock(&queue->mutex); */
/*             return ENOMEM; */
/*         } */
/*         const size_t i = */
/*             (queue->start_current + aqueue_len(queue)) % queue->capacity; */
/*         buf_set_at(queue->data, queue->capacity, item, i); */
/*         __atomic_fetch_add(&queue->len, 1, __ATOMIC_ACQUIRE); */
/*     } */

/*     PG_ASSERT_COND(aqueue_len(queue), <=, queue->capacity, "%zu"); */
/*     PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu"); */

/*     pthread_mutex_unlock(&queue->mutex); */

/*     return 0; */
/* } */

/* int aqueue_pop(struct aqueue* queue, aqueue_data_t* item) { */
/*     PG_ASSERT_NOT_EQ(queue, NULL, "%p"); */
/*     PG_ASSERT_NOT_EQ(queue->data, NULL, "%p"); */
/*     PG_ASSERT_NOT_EQ(item, NULL, "%p"); */

/*     int ret; */
/*     while ((ret = pthread_mutex_trylock(&queue->mutex)) == EBUSY) { */
/*     } */
/*     if (ret != 0) return ret; */

/*     { */
/*         PG_ASSERT_COND(aqueue_len(queue), <=, queue->capacity, "%zu"); */
/*         PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu"); */

/*         if (aqueue_len(queue) == 0) { */
/*             pthread_mutex_unlock(&queue->mutex); */
/*             return EINVAL; */
/*         } */

/*         buf_get_at(queue->data, queue->capacity, *item,
 * queue->start_current); */

/*         __atomic_fetch_add(&queue->len, -1, __ATOMIC_ACQUIRE); */
/*         queue->start_current = (queue->start_current + 1) % queue->capacity;
 */
/*     } */

/*     PG_ASSERT_COND(aqueue_len(queue), <=, queue->capacity, "%zu"); */
/*     PG_ASSERT_COND(queue->start_current, <=, queue->capacity, "%zu"); */

/*     pthread_mutex_unlock(&queue->mutex); */

/*     return 0; */
/* } */

