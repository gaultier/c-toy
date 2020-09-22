#pragma once
#include "thread_pool.h"

struct actor_system;
struct actor;

struct actor {
    size_t id;
    size_t thread_id;
    work_fn main;
    struct aqueue message_queue;
    struct actor_system* actor_system;
    struct thread_pool_work_item* work;
};

struct actor_msg {
    size_t sender_id;
    size_t receiver_id;
    void* data;
};

struct actor_system {
    struct actor* actors;
    struct aqueue central_message_queue;
    struct thread_pool pool;
    struct allocator* allocator;
};

int actor_init(struct actor* actor, work_fn main,
               struct actor_system* actor_system) {
    PG_ASSERT_NOT_EQ(actor, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor_system, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor_system->allocator, NULL, "%p");

    actor->work = actor_system->allocator->realloc(
        NULL, sizeof(struct thread_pool_work_item));
    if (actor->work == NULL) return ENOMEM;

    actor->work->arg = NULL;
    actor->work->fn = main;
    actor->id = 0;
    actor->thread_id = 0;
    actor->actor_system = actor_system;

    int err;
    if ((err = thread_pool_push(&actor_system->pool, actor->work)) != 0)
        return err;

    if ((err = aqueue_init(&actor->message_queue,
                                      actor_system->allocator)) != 0)
        return err;

    return 0;
}

void actor_deinit(struct actor* actor) {
    if (actor == NULL) return;

    PG_ASSERT_NOT_EQ(actor, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor->actor_system, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor->actor_system->allocator, NULL, "%p");

    if (actor->work != NULL) actor->actor_system->allocator->free(actor->work);

    aqueue_deinit(&actor->message_queue);  // FIXME: was it init-ed?
}

int actor_system_init(struct actor_system* actor_system,
                      struct allocator* allocator) {
    PG_ASSERT_NOT_EQ(actor_system, NULL, "%p");
    PG_ASSERT_NOT_EQ(allocator, NULL, "%p");

    actor_system->actors = NULL;
    actor_system->allocator = allocator;

    int err;
    if ((err = aqueue_init(&actor_system->central_message_queue,
                                      allocator)) != 0)
        return err;

    PG_ASSERT_EQ(thread_pool_init(&actor_system->pool, 4, allocator), 0,
                 "%d");  // FIXME: nproc

    thread_pool_start(&actor_system->pool);

    return 0;
}

void actor_system_deinit(struct actor_system* actor_system) {
    if (actor_system == NULL) return;

    PG_ASSERT_NOT_EQ(actor_system, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor_system->allocator, NULL, "%p");

    thread_pool_wait_until_finished(&actor_system->pool);

    if (actor_system->actors) buf_free(actor_system->actors);
    aqueue_deinit(&actor_system->central_message_queue);

    thread_pool_deinit(&actor_system->pool);
}

int actor_send_message(struct actor* sender, struct actor_msg* msg) {
    PG_ASSERT_NOT_EQ(sender, NULL, "%p");
    PG_ASSERT_NOT_EQ(msg, NULL, "%p");
    PG_ASSERT_NOT_EQ(sender->actor_system, NULL, "%p");
    PG_ASSERT_NOT_EQ(sender->actor_system->actors, NULL, "%p");

    for (size_t i = 0; i < buf_size(sender->actor_system->actors); i++) {
        struct actor* actor = &sender->actor_system->actors[i];
        if (actor->id == msg->receiver_id) {
            return aqueue_push(&actor->message_queue,
                                          &msg);  // FIXME !!!
        }
    }
    return EINVAL;
}

int actor_receive_message(struct actor* actor, struct actor_msg** msg) {
    return aqueue_pop(&actor->message_queue, (void**)msg);
}
