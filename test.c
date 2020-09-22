#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "aqueue.h"

void* push(void* arg) {
    struct aqueue* queue = arg;
    for (size_t i = 0; i < 500; i++) {
        printf("Auxiliary thread: popping\n");
        int val = (int)aqueue_pop(queue);
        printf("Auxiliary thread: popped %d\n", val);
    }
    return NULL;
}

int main() {
    struct aqueue queue = {0};
    /* pthread_t thread; */
    /* pthread_create(&thread, NULL, push, &queue); */

    for (size_t i = 0; i < 1; i++) {
        struct aqueue_node node = {.data = (void*)i};
        printf("Main thread: pushing %zu\n", i);
        aqueue_push(&queue, &node);
        printf("Main thread: pushed %zu\n", i);

        int val = (int)aqueue_pop(&queue);
        printf("Main thread: pop = %d\n", val);
    }
    /* pthread_join(thread, NULL); */
}
