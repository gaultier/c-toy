#include "thread_safe_queue.h"

#include <criterion/criterion.h>
#include <stdlib.h>

Test(thread_safe_queue, init) {
    struct thread_safe_queue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect(thread_safe_queue_init(&queue, &allocator) == 0);
}

Test(thread_safe_queue, pop_empty) {
    struct thread_safe_queue queue;
    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect(thread_safe_queue_init(&queue, &allocator) == 0);

    data_t item;
    cr_expect(thread_safe_queue_pop(&queue, &item) == EINVAL);
}
