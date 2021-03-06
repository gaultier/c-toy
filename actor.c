#include "actor.h"

#include <unistd.h>

#define MSG_PING 1
#define MSG_PONG 2
int ping_data = 1;
int pong_data = 2;

int actor_ping_id = 0;
int actor_pong_id = 0;

int ping_pong_counter = 0;

void fn_ping(void* arg) {
    PG_ASSERT_NOT_EQ(arg, NULL, "%p");

    struct actor* self = arg;

    struct actor_msg* msg = NULL;
    while (__atomic_load_n(&ping_pong_counter, __ATOMIC_ACQUIRE) < 5) {
        actor_receive_message(self, &msg);
        if (msg == NULL) continue;

        printf("actor #%zu: Received message from actor #%zu\n", self->id,
               msg->sender_id);

        PG_ASSERT_NOT_EQ(msg->data, NULL, "%p");
        switch (*((int*)msg->data)) {
            case MSG_PING:
                puts("PING");
                __atomic_fetch_add(&ping_pong_counter, 1, __ATOMIC_ACQUIRE);
                actor_send_message(self, actor_pong_id, &pong_data);
                break;
        }

        self->actor_system->allocator->free(msg);
    }
    printf("actor #%zu finished\n", self->id);
}

void fn_pong(void* arg) {
    PG_ASSERT_NOT_EQ(arg, NULL, "%p");

    struct actor* self = arg;

    struct actor_msg* msg = NULL;
    while (__atomic_load_n(&ping_pong_counter, __ATOMIC_ACQUIRE) < 5) {
        actor_receive_message(self, &msg);
        if (msg == NULL) continue;

        printf("actor #%zu: Received message from actor #%zu\n", self->id,
               msg->sender_id);

        PG_ASSERT_NOT_EQ(msg->data, NULL, "%p");
        switch (*((int*)msg->data)) {
            case MSG_PONG:
                puts("PONG");
                __atomic_fetch_add(&ping_pong_counter, 1, __ATOMIC_ACQUIRE);
                actor_send_message(self, actor_ping_id, &ping_data);
                break;
        }
        self->actor_system->allocator->free(msg);
    }
    printf("actor #%zu finished\n", self->id);
}

int main() {
    struct allocator allocator = {.realloc = realloc, .free = free};

    struct actor_system actor_system;
    PG_ASSERT_EQ(actor_system_init(&actor_system, &allocator), 0, "%d");

    struct actor actor_ping;
    PG_ASSERT_EQ(actor_init(&actor_ping, fn_ping, &actor_system), 0, "%d");
    printf("Ping: id=%zu\n", actor_ping.id);
    actor_ping_id = actor_ping.id;

    struct actor actor_pong;
    PG_ASSERT_EQ(actor_init(&actor_pong, fn_pong, &actor_system), 0, "%d");
    printf("Pong: id=%zu\n", actor_pong.id);
    actor_pong_id = actor_pong.id;

    PG_ASSERT_EQ(actor_send_message(&actor_ping, actor_pong.id, &pong_data), 0,
                 "%d");

    thread_pool_wait_until_finished(&actor_system.pool);
    /* actor_system_deinit(&actor_system); */
}
