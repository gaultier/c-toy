#include "thread_safe_queue.h"

#include <criterion/criterion.h>
#include <stdlib.h>

Test(thread_safe_queue, init) {
    struct thread_safe_queue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(thread_safe_queue_init(&queue, &allocator), 0);
}

Test(thread_safe_queue, pop_empty) {
    struct thread_safe_queue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(thread_safe_queue_init(&queue, &allocator), 0);

    data_t item;
    cr_expect_eq(thread_safe_queue_pop(&queue, &item), EINVAL);
    cr_expect_eq(queue.len, 0);
}

Test(thread_safe_queue, push) {
    struct thread_safe_queue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(thread_safe_queue_init(&queue, &allocator), 0);

    int item = 1;
    cr_expect_eq(thread_safe_queue_push(&queue, &item), 0);
    cr_expect_eq(queue.len, 1);
}

Test(thread_safe_queue, push_pop) {
    struct thread_safe_queue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(thread_safe_queue_init(&queue, &allocator), 0);

    // Push
    int pushed = 1;
    { cr_expect_eq(thread_safe_queue_push(&queue, &pushed), 0); }

    // Pop
    {
        void *popped = NULL;
        cr_expect_eq(thread_safe_queue_pop(&queue, &popped), 0);
        cr_expect_eq(*((int *)popped), 1);
    }
}

Test(thread_safe_queue, push_nomem) {
    struct thread_safe_queue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(thread_safe_queue_init(&queue, &allocator), 0);

    // Push
    int items[1000];
    for (size_t i = 0; i < 1000; i++) {
        items[i] = i;
        cr_expect_eq(thread_safe_queue_push(&queue, &items[i]), 0);
    }

    cr_expect_eq(thread_safe_queue_push(&queue, &items[0]), ENOMEM);
}
