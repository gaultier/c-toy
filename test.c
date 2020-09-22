#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "aqueue.h"

/* void* push(void* arg) { */
/*     struct aqueue* queue = arg; */
/*     for (size_t i = 0; i < 500; i++) { */
/*         printf("Auxiliary thread: popping\n"); */
/*         int val = (int)aqueue_pop(queue); */
/*         printf("Auxiliary thread: popped %d\n", val); */
/*     } */
/*     return NULL; */
/* } */

int main() {
    struct aqueue queue = {0};
    /* pthread_t thread; */
    /* pthread_create(&thread, NULL, push, &queue); */

    int vals[AQUEUE_CAPACITY] = {0};
    for (size_t i = 0; i < 500; i++) {
        vals[i] = i;
        struct aqueue_node node = {.data = &vals[i]};
        printf("Main thread: pushing %zu\n", i);
        aqueue_push(&queue, &node);
        printf("Main thread: pushed=%zu front=%zu rear=%zu\n", i, queue.front,
               queue.rear);

        int* val = aqueue_pop(&queue);
        printf("Main thread: pop = %d\n", *val);
    }
    /* pthread_join(thread, NULL); */
}
