#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "aqueue.h"

int vals[AQUEUE_CAPACITY] = {0};

void* push(void* arg) {
    struct aqueue* queue = arg;
    for (size_t i = 500; i < AQUEUE_CAPACITY; i++) {
        vals[i] = i;
        printf("Auxiliary thread: pushing %zu\n", i);
        aqueue_push(queue, &vals[i]);
        printf("Auxiliary thread: pushed=%zu\n", i);

        int* val = aqueue_pop(queue);
        printf("Auxiliary thread: pop = %d\n", *val);
    }
    return NULL;
}

int main() {
    struct aqueue queue = {0};
    pthread_t thread;
    pthread_create(&thread, NULL, push, &queue);

    for (size_t i = 0; i < 501; i++) {
        vals[i] = i;
        printf("Main thread: pushing %zu\n", i);
        aqueue_push(&queue, &vals[i]);
        printf("Main thread: pushed=%zu \n", i);

        int* val = aqueue_pop(&queue);
        printf("Main thread: pop = %d\n", *val);
    }
    pthread_join(thread, NULL);
}
