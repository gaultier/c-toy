#pragma once
#include "thread_pool.h"

struct actor {
    size_t id;
    size_t thread_id;
    work_fn main;
    struct thread_pool_work_item* work;
};

int actor_init(struct thread_pool* pool, work_fn main, struct actor* actor,
               struct allocator* allocator) {
    PG_ASSERT_NOT_EQ(pool, NULL, "%p");
    PG_ASSERT_NOT_EQ(allocator, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor, NULL, "%p");

    actor->work =
        allocator->realloc(NULL, sizeof(struct thread_pool_work_item));
    if (actor->work == NULL) return ENOMEM;

    actor->work->arg = NULL;
    actor->work->fn = main;
    actor->id = 0;
    actor->thread_id = 0;

    int err;
    if ((err = thread_pool_push(pool, actor->work)) != 0) return err;

    return 0;
}

void actor_deinit(struct actor* actor, struct allocator* allocator) {
    PG_ASSERT_NOT_EQ(allocator, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor, NULL, "%p");

    if (actor->work != NULL) allocator->free(actor->work);
}
