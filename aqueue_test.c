#include "aqueue.h"

#include <criterion/criterion.h>
#include <pthread.h>

Test(aqueue, push_pop_single) {
    struct aqueue queue = {0};

    int val = 99;
    cr_expect_eq(aqueue_len(&queue), 0);
    cr_expect_eq(aqueue_push(&queue, &val), 0);
    cr_expect_eq(aqueue_len(&queue), 1);

    int* popped = aqueue_pop(&queue);
    cr_expect_neq(popped, NULL);
    cr_expect_eq(*popped, 99);

    cr_expect_eq(aqueue_pop(&queue), NULL);
}

int vals[AQUEUE_CAPACITY] = {0};

void* push_pop(void* arg) {
    struct aqueue* queue = arg;
    for (size_t i = 500; i < AQUEUE_CAPACITY; i++) {
        vals[i] = i;
        cr_expect_eq(aqueue_push(queue, &vals[i]), 0);
        aqueue_pop(queue);
    }
    return NULL;
}

Test(aqueue, push_pop_multi) {
    struct aqueue queue = {0};

    pthread_t thread;
    pthread_create(&thread, NULL, push_pop, &queue);

    for (size_t i = 0; i < 500; i++) {
        vals[i] = i;
        cr_expect_eq(aqueue_push(&queue, &vals[i]), 0);
        aqueue_pop(&queue);
    }
    pthread_join(thread, NULL);
}
