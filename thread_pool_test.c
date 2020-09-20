#include "thread_pool.h"

#include <criterion/criterion.h>
#include <stdlib.h>

Test(thread_pool, init_deinit) {
    struct thread_pool pool;
    struct allocator allocator = {.realloc = realloc, .free = free};
    cr_expect_eq(thread_pool_init(&pool, 4, &allocator), 0);
    thread_pool_deinit(&pool, &allocator);
}

Test(thread_pool, start_stop) {
    struct thread_pool pool;
    struct allocator allocator = {.realloc = realloc, .free = free};
    cr_expect_eq(thread_pool_init(&pool, 4, &allocator), 0);

    thread_pool_start(&pool);
    thread_pool_stop(&pool);
    thread_pool_deinit(&pool, &allocator);
}

void incr(void* arg) {
    int* count = arg;
    __atomic_fetch_add(count, 1, __ATOMIC_ACQUIRE);
}

Test(thread_pool, run_stop) {
    struct thread_pool pool;
    struct allocator allocator = {.realloc = realloc, .free = free};
    cr_expect_eq(thread_pool_init(&pool, 4, &allocator), 0);

    int count = 0;
    struct work_item work_a = {.arg = &count, .fn = incr};
    cr_expect_eq(thread_pool_push(&pool, &work_a), 0);

    thread_pool_start(&pool);

    struct work_item work_b = {.arg = &count, .fn = incr};
    cr_expect_eq(thread_pool_push(&pool, &work_b), 0);

    struct work_item work_c = {.arg = &count, .fn = incr};
    cr_expect_eq(thread_pool_push(&pool, &work_c), 0);

    while (count != 3) {
    }
    thread_pool_stop(&pool);
    thread_pool_deinit(&pool, &allocator);

    cr_expect_eq(count, 3);
}
