#include "thread_pool.h"

#include <criterion/criterion.h>
#include <stdlib.h>

Test(thread_pool, init_deinit) {
    struct thread_pool pool;
    struct allocator allocator = {.realloc = realloc, .free = free};
    cr_expect_eq(thread_pool_init(&pool, 4, &allocator), 0);
    thread_pool_deinit(&pool, &allocator);
}

Test(thread_pool, run_stop) {
    struct thread_pool pool;
    struct allocator allocator = {.realloc = realloc, .free = free};
    cr_expect_eq(thread_pool_init(&pool, 4, &allocator), 0);

    thread_pool_start(&pool);
    thread_pool_stop(&pool);
    thread_pool_deinit(&pool, &allocator);
}
