#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "thread_pool.h"

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

    struct thread_pool pool;
    struct allocator allocator = {.realloc = realloc, .free = free};
    PG_ASSERT_EQ(thread_pool_init(&pool, 4, &allocator), 0, "%d");

    thread_pool_start(&pool);
    sleep(5);
    thread_pool_stop(&pool);
    thread_pool_deinit(&pool, &allocator);
}
