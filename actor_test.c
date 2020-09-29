
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
            default:
                cr_assert_fail("unexpected message");
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
    printf("single_actor: %zu\n", actor_ping.id);

    cr_expect_eq(actor_send_message(&actor_ping, actor_ping.id, &ping_data), 0);

    thread_pool_wait_until_finished(&actor_system.pool);

    cr_expect_eq(received_ping, 1);

    actor_system_deinit(&actor_system);
}

#define MSG_PONG 2
int ping_pong_counter = 0;
int ping_actor_id = 0;
int pong_actor_id = 0;
int pong_data = 2;

void fn_counter_ping(void* arg) {
    PG_ASSERT_NOT_EQ(arg, NULL, "%p");

    struct actor* self = arg;

    struct actor_msg* msg = NULL;
    while (__atomic_load_n(&ping_pong_counter, __ATOMIC_ACQUIRE) < 5) {
        actor_receive_message(self, &msg);
        if (msg == NULL) continue;

        PG_ASSERT_NOT_EQ(msg->data, NULL, "%p");
        switch (*((int*)msg->data)) {
            case MSG_PING:
                __atomic_fetch_add(&ping_pong_counter, 1, __ATOMIC_ACQUIRE);
                actor_send_message(self, pong_actor_id, &pong_data);
                break;
            default:
                cr_assert_fail("unexpected message");
        }

        self->actor_system->allocator->free(msg);
    }
}

void fn_counter_pong(void* arg) {
    PG_ASSERT_NOT_EQ(arg, NULL, "%p");

    struct actor* self = arg;

    struct actor_msg* msg = NULL;
    while (__atomic_load_n(&ping_pong_counter, __ATOMIC_ACQUIRE) < 5) {
        actor_receive_message(self, &msg);
        if (msg == NULL) continue;

        PG_ASSERT_NOT_EQ(msg->data, NULL, "%p");
        switch (*((int*)msg->data)) {
            case MSG_PONG:
                __atomic_fetch_add(&ping_pong_counter, 1, __ATOMIC_ACQUIRE);
                actor_send_message(self, ping_actor_id, &ping_data);
                break;
            default:
                cr_assert_fail("unexpected message");
        }

        self->actor_system->allocator->free(msg);
    }
}

Test(actor, ping_pong) {
    struct allocator allocator = {.realloc = realloc, .free = free};

    struct actor_system actor_system;
    cr_expect_eq(actor_system_init(&actor_system, &allocator), 0);

    struct actor actor_ping;
    cr_expect_eq(actor_init(&actor_ping, fn_counter_ping, &actor_system), 0);
    ping_actor_id = actor_ping.id;
    printf("ping_actor_id: %d\n", ping_actor_id);

    struct actor actor_pong;
    cr_expect_eq(actor_init(&actor_pong, fn_counter_pong, &actor_system), 0);
    pong_actor_id = actor_pong.id;
    printf("pong_actor_id: %d\n", pong_actor_id);

    cr_expect_eq(actor_send_message(&actor_ping, actor_pong.id, &pong_data), 0);
    /* printf("actor_send_message: send=%zu receiver=%zu kind=%zu\n",
     * actor_ping.id, a) */

    thread_pool_wait_until_finished(&actor_system.pool);

    cr_expect_eq(ping_pong_counter, 5);

    actor_system_deinit(&actor_system);
}
