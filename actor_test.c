
#include "actor.h"

#include <criterion/criterion.h>

Test(actor, init_deinit) {
    struct allocator allocator = {.realloc = realloc, .free = free};

    struct actor_system actor_system;
    cr_expect_eq(actor_system_init(&actor_system, &allocator), 0);

    thread_pool_wait_until_finished(&actor_system.pool);
    actor_system_deinit(&actor_system);
}

#define MSG_PING 1
int ping_data = 1;
int received_ping = 0;

void one_shot_ping(void* arg) {
    PG_ASSERT_NOT_EQ(arg, NULL, "%p");

    struct actor* self = arg;

    struct actor_msg* msg = NULL;
    while (received_ping == 0) {
        actor_receive_message(self, &msg);
        if (msg == NULL) continue;

        PG_ASSERT_NOT_EQ(msg->data, NULL, "%p");
        switch (*((int*)msg->data)) {
            case MSG_PING:
                received_ping = 1;
                break;
        }

        self->actor_system->allocator->free(msg);
    }
}

Test(actor, single_actor) {
    struct allocator allocator = {.realloc = realloc, .free = free};

    struct actor_system actor_system;
    cr_expect_eq(actor_system_init(&actor_system, &allocator), 0);

    struct actor actor_ping;
    cr_expect_eq(actor_init(&actor_ping, one_shot_ping, &actor_system), 0);

    actor_send_message(&actor_ping, actor_ping.id, &ping_data);

    thread_pool_wait_until_finished(&actor_system.pool);

    cr_expect_eq(received_ping, 1);

    actor_system_deinit(&actor_system);
}

int ping_pong_counter = 0;

Test(actor, ping_pong) {
    struct allocator allocator = {.realloc = realloc, .free = free};

    struct actor_system actor_system;
    cr_expect_eq(actor_system_init(&actor_system, &allocator), 0);

    struct actor actor_ping;
    cr_expect_eq(actor_init(&actor_ping, fn_ping, &actor_system), 0);

    struct actor actor_pong;
    cr_expect_eq(actor_init(&actor_pong, fn_pong, &actor_system), 0);

    actor_send_message(&actor_ping, actor_ping.id, &ping_data);

    thread_pool_wait_until_finished(&actor_system.pool);

    cr_expect_eq(received_ping, 1);

    actor_system_deinit(&actor_system);
}
