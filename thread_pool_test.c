#include "thread_pool.h"

#include <criterion/criterion.h>

Test(thread_pool, init_deinit) {
    struct thread_pool pool;
    cr_expect_eq(thread_pool_init(&pool, 4), 0);
    thread_pool_deinit(&pool);
}

Test(thread_pool, start_stop) {
    struct thread_pool pool;
    cr_expect_eq(thread_pool_init(&pool, 4), 0);

    thread_pool_start(&pool);
    thread_pool_stop(&pool);
    thread_pool_deinit(&pool);
}

void incr(void* arg) {
    int* count = arg;
    __atomic_fetch_add(count, 1, __ATOMIC_ACQUIRE);
}

Test(thread_pool, run_3) {
    struct thread_pool pool;
    cr_expect_eq(thread_pool_init(&pool, 4), 0);

    int count = 0;
    struct thread_pool_work_item work_a = {.arg = &count, .fn = incr};
    cr_expect_eq(thread_pool_push(&pool, &work_a), 0);

    thread_pool_start(&pool);

    struct thread_pool_work_item work_b = {.arg = &count, .fn = incr};
    cr_expect_eq(thread_pool_push(&pool, &work_b), 0);

    struct thread_pool_work_item work_c = {.arg = &count, .fn = incr};
    cr_expect_eq(thread_pool_push(&pool, &work_c), 0);

    thread_pool_wait_until_finished(&pool);
    thread_pool_deinit(&pool);

    cr_expect_eq(count, 3);
}

Test(thread_pool, run_n) {
    struct thread_pool pool;
    cr_expect_eq(thread_pool_init(&pool, 4), 0);

    int count = 0;
    struct thread_pool_work_item work[400];
    for (size_t i = 0; i < 400; i++) {
        work[i].arg = &count;
        work[i].fn = incr;
        cr_expect_eq(thread_pool_push(&pool, &work[i]), 0);
    }

    thread_pool_start(&pool);

    thread_pool_wait_until_finished(&pool);
    thread_pool_deinit(&pool);

    cr_expect_eq(count, 400);
}
