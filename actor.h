#pragma once
#include "allocator.h"
#include "thread_pool.h"

struct actor_system;
struct actor;

struct actor {
    size_t id;
    //    size_t thread_id;  // Unused for now, pin on a thread for later
    work_fn main;
    struct aqueue message_queue;
    struct actor_system* actor_system;
    struct thread_pool_work_item work;
};

struct actor_msg {
    size_t sender_id;
    size_t receiver_id;
    void* data;
    struct actor* receiver;
};

struct actor_system {
    struct actor** actors;
    struct thread_pool pool;
    struct allocator* allocator;
};

static size_t id = 1;

int actor_init(struct actor* actor, work_fn main,
               struct actor_system* actor_system) {
    PG_ASSERT_NOT_EQ(actor, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor_system, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor_system->allocator, NULL, "%p");

    actor->work.arg = actor;
    actor->work.fn = main;
    actor->id = __atomic_fetch_add(&id, 1, __ATOMIC_ACQUIRE);
    actor->actor_system = actor_system;

    int err;
    if ((err = thread_pool_push(&actor_system->pool, &actor->work)) != 0)
        return err;

    struct aqueue_node* nodes = actor_system->allocator->realloc(
        NULL, sizeof(struct aqueue_node) * 1000);
    if (nodes == NULL) return ENOMEM;

    aqueue_init(&actor->message_queue, nodes, 1000);

    buf_push(actor->actor_system->actors, actor);

    return 0;
}

void actor_deinit(struct actor* actor) {
    if (actor == NULL) return;

    PG_ASSERT_NOT_EQ(actor, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor->actor_system, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor->actor_system->allocator, NULL, "%p");

    if (actor->message_queue.nodes != NULL)
        actor->actor_system->allocator->free(actor->message_queue.nodes);
}

int actor_system_init(struct actor_system* actor_system,
                      struct allocator* allocator) {
    PG_ASSERT_NOT_EQ(actor_system, NULL, "%p");
    PG_ASSERT_NOT_EQ(allocator, NULL, "%p");

    actor_system->actors = NULL;
    actor_system->allocator = allocator;

    int err;

    // FIXME: nproc
    if ((err = thread_pool_init(&actor_system->pool, 4)) != 0) return err;

    thread_pool_start(&actor_system->pool);

    return 0;
}

/* struct actor* actor_system_spawn(struct actor_system* actor_system, */
/*                                  work_fn main) { */
/*     struct actor* actor = malloc(sizeof(struct actor)); */
/*     if (actor != NULL) return NULL; */

/*     actor_init(actor, main, actor_system); */
/*     return actor; */
/* } */

void actor_system_deinit(struct actor_system* actor_system) {
    if (actor_system == NULL) return;

    PG_ASSERT_NOT_EQ(actor_system, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor_system->allocator, NULL, "%p");

    if (actor_system->actors) {
        for (size_t i = 0; i < buf_size(actor_system->actors); i++)
            actor_deinit(actor_system->actors[i]);
        // TODO: should we free(actor) ?

        buf_free(actor_system->actors);
    }

    thread_pool_deinit(&actor_system->pool);
}

int actor_receive_message(struct actor* actor, struct actor_msg** msg) {
    PG_ASSERT_NOT_EQ(actor, NULL, "%p");
    PG_ASSERT_NOT_EQ(msg, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor->message_queue.nodes, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor->message_queue.capacity, (size_t)0, "%zu");

    *msg = aqueue_pop(&actor->message_queue);
    return (*msg == NULL);
}

void actor_system_run(struct actor_system* actor_system) {
    PG_ASSERT_NOT_EQ(actor_system, NULL, "%p");
    PG_ASSERT_NOT_EQ(actor_system->allocator, NULL, "%p");

    while (1) {
        for (size_t i = 0; i < buf_size(actor_system->actors); i++) {
            struct actor* receiver = actor_system->actors[i];
            PG_ASSERT_NOT_EQ(receiver, NULL, "%p");
            PG_ASSERT_NOT_EQ(receiver->main, NULL, "%p");

            struct actor_msg* msg = NULL;

            actor_receive_message(receiver, &msg);

            if (msg == NULL) continue;

            receiver->main(msg);

            actor_system->allocator->free(msg);
        }
    }
}

int actor_send_message(struct actor* sender, size_t receiver_id, void* data) {
    PG_ASSERT_NOT_EQ(sender, NULL, "%p");
    PG_ASSERT_NOT_EQ(sender->actor_system, NULL, "%p");
    PG_ASSERT_NOT_EQ(sender->actor_system->actors, NULL, "%p");

    struct actor_msg* msg = realloc(NULL, sizeof(struct actor));
    if (msg == NULL) return ENOMEM;

    msg->receiver_id = receiver_id;
    msg->sender_id = sender->id;
    msg->data = data;

    printf("actor_send_message: sender_id=%zu receiver_id=%zu\n",
           msg->sender_id, msg->receiver_id);

    for (size_t i = 0; i < buf_size(sender->actor_system->actors); i++) {
        struct actor* actor = sender->actor_system->actors[i];
        if (actor->id == msg->receiver_id) {
            msg->receiver = actor;
            return aqueue_push(&actor->message_queue, msg);
        }
    }
    return EINVAL;
}

