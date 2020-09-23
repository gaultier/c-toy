#include "actor.h"

#include <unistd.h>

void print(void* arg) {
    PG_ASSERT_NOT_EQ(arg, NULL, "%p");

    struct actor* self = arg;

    struct actor_msg* msg = NULL;
    int err;
    while ((err = actor_receive_message(self, &msg)) == 0) {
        PG_ASSERT_NOT_EQ(msg, NULL, "%p");

        printf("actor #%zu: Received message from actor #%zu\n", self->id,
               msg->sender_id);

        self->actor_system->allocator->free(msg);
    }
}

int main() {
    struct allocator allocator = {.realloc = realloc, .free = free};

    struct actor_system actor_system;
    PG_ASSERT_EQ(actor_system_init(&actor_system, &allocator), 0, "%d");

    struct actor actor;
    PG_ASSERT_EQ(actor_init(&actor, print, &actor_system), 0, "%d");
    printf("Actor: id=%zu\n", actor.id);

    int val = 99;
    PG_ASSERT_EQ(actor_send_message(&actor, 1, &val), 0, "%d");

    actor_system_deinit(&actor_system);
}
