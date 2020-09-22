#include "actor.h"

#include <unistd.h>

/* #include "aco.h" */

/* void ping() { */
/* int count = 0; */
/* while (count++ < 5) { */
/*     puts("ping"); */
/*     aco_yield(); */
/* } */
/* aco_exit(); */
/* } */

/* void pong() { */
/* int count = 0; */
/* while (count++ < 5) { */
/*     puts("pong"); */
/*     aco_yield(); */
/* } */
/* aco_exit(); */
/* } */

void print(void* arg) {
    struct actor* self = arg;

    struct actor_msg* msg = NULL;
    while (1) {
        if (actor_receive_message(self, &msg) != 0) {
            pg_nanosleep(100);
            continue;
        }

        PG_ASSERT_NOT_EQ(msg, NULL, "%p");

        printf("actor #%zu: Received message from actor #%zu\n", self->id,
               msg->sender_id);

        self->actor_system->allocator->free(msg);
    }
}

int main() {
    /* aco_thread_init(NULL); */

    /* aco_t* main_co = aco_create(NULL, NULL, 0, NULL, NULL); */

    /* aco_share_stack_t* sstk = aco_share_stack_new(0); */
    /* int ping_co_ct_arg_point_to_me = 0; */
    /* aco_t* ping_co = */
    /*     aco_create(main_co, sstk, 0, ping, &ping_co_ct_arg_point_to_me); */

    /* int pong_co_ct_arg_point_to_me = 0; */
    /* aco_t* pong_co = */
    /*     aco_create(main_co, sstk, 0, pong, &pong_co_ct_arg_point_to_me); */

    /* while (!ping_co->is_end && !pong_co->is_end) { */
    /*     aco_resume(ping_co); */
    /*     aco_resume(pong_co); */
    /* } */

    /* puts("The End"); */

    /* aco_destroy(pong_co); */
    /* pong_co = NULL; */
    /* aco_destroy(ping_co); */
    /* ping_co = NULL; */
    /* // destroy the share stack sstk. */
    /* aco_share_stack_destroy(sstk); */
    /* sstk = NULL; */
    /* // destroy the main_co. */
    /* aco_destroy(main_co); */
    /* main_co = NULL; */

    struct allocator allocator = {.realloc = realloc, .free = free};

    struct actor_system actor_system;
    PG_ASSERT_EQ(actor_system_init(&actor_system, &allocator), 0, "%d");

    struct actor actor;
    PG_ASSERT_EQ(actor_init(&actor, print, &actor_system), 0, "%d");

    actor_deinit(&actor);
    actor_system_deinit(&actor_system);
}
