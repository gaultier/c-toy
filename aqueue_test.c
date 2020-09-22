#include "aqueue.h"

#include <criterion/criterion.h>

Test(buf, buf_get_at) {
    int data[5] = {1, 2, 3, 4, 5};
    int val = 0;
    buf_get_at(data, (size_t)5, val, (size_t)4);

    cr_expect_eq(val, 5);
}

Test(aqueue, init) {
    struct aqueue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(aqueue_init(&queue, &allocator), 0);
    aqueue_deinit(&queue);
}

Test(aqueue, pop_empty) {
    struct aqueue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(aqueue_init(&queue, &allocator), 0);

    aqueue_data_t item;
    cr_expect_eq(aqueue_pop(&queue, &item), EINVAL);
    cr_expect_eq(queue.len, 0);

    aqueue_deinit(&queue);
}

Test(aqueue, push) {
    struct aqueue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(aqueue_init(&queue, &allocator), 0);

    int item = 1;
    cr_expect_eq(aqueue_push(&queue, &item), 0);
    cr_expect_eq(queue.len, 1);

    aqueue_deinit(&queue);
}

Test(aqueue, push_pop) {
    struct aqueue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(aqueue_init(&queue, &allocator), 0);

    // Push
    int pushed = 1;
    {
        cr_expect_eq(aqueue_push(&queue, &pushed), 0);
        cr_expect_eq(queue.len, 1);
    }

    // Pop
    {
        void *popped = NULL;
        cr_expect_eq(aqueue_pop(&queue, &popped), 0);
        cr_expect_eq(*((int *)popped), 1);
        cr_expect_eq(queue.len, 0);
    }

    aqueue_deinit(&queue);
}

Test(aqueue, push_nomem) {
    struct aqueue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(aqueue_init(&queue, &allocator), 0);

    // Push
    int items[1000];
    for (size_t i = 0; i < 1000; i++) {
        items[i] = i;
        cr_expect_eq(aqueue_push(&queue, &items[i]), 0);
        cr_expect_eq(queue.len, i + 1);
    }

    cr_expect_eq(aqueue_push(&queue, &items[0]), ENOMEM);
    cr_expect_eq(queue.len, 1000);

    cr_expect_eq(aqueue_push(&queue, &items[0]), ENOMEM);
    cr_expect_eq(queue.len, 1000);

    void *popped = NULL;
    cr_expect_eq(aqueue_pop(&queue, &popped), 0);
    cr_expect_eq(queue.len, 999);
    cr_expect_eq(aqueue_push(&queue, &items[0]), 0);
    cr_expect_eq(queue.len, 1000);

    aqueue_deinit(&queue);
}

void *pop(void *arg) {
    struct aqueue *queue = arg;

    void *popped = NULL;
    for (size_t i = 0; i < 500; i++)
        cr_expect_eq(aqueue_pop(queue, &popped), 0);

    return NULL;
}

Test(aqueue, pop_multi) {
    struct aqueue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(aqueue_init(&queue, &allocator), 0);

    // Push
    int items[1000];
    for (size_t i = 0; i < 1000; i++) {
        items[i] = i;
        cr_expect_eq(aqueue_push(&queue, &items[i]), 0);
        cr_expect_eq(queue.len, i + 1);
    }

    pthread_t thread;
    pthread_create(&thread, NULL, pop, &queue);

    void *popped = NULL;
    for (size_t i = 0; i < 500; i++)
        cr_expect_eq(aqueue_pop(&queue, &popped), 0);

    pthread_join(thread, NULL);

    cr_expect_eq(queue.len, 0);

    aqueue_deinit(&queue);
}

void *push_pop(void *arg) {
    struct aqueue *queue = arg;

    void *popped = NULL;
    static int item = 99;
    while (queue->len != 0) {
        cr_expect_eq(aqueue_pop(queue, &popped), 0);
        if (aqueue_pop(queue, &popped) == EINVAL) break;
        cr_expect_eq(aqueue_push(queue, &item), 0);
    }

    return NULL;
}

Test(aqueue, push_pop_multi) {
    struct aqueue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(aqueue_init(&queue, &allocator), 0);

    // Push
    int items[1000];
    for (size_t i = 0; i < 1000; i++) {
        items[i] = i;
        cr_expect_eq(aqueue_push(&queue, &items[i]), 0);
        cr_expect_eq(queue.len, i + 1);
    }

    pthread_t thread;
    pthread_create(&thread, NULL, pop, &queue);

    void *popped = NULL;
    int item = 42;
    while (queue.len != 0) {
        cr_expect_eq(aqueue_pop(&queue, &popped), 0);
        if (aqueue_pop(&queue, &popped) == EINVAL) break;
        cr_expect_eq(aqueue_push(&queue, &item), 0);
    }

    pthread_join(thread, NULL);

    cr_expect_eq(queue.len, 0);

    aqueue_deinit(&queue);
}
