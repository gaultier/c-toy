#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "aqueue.h"

void* push(void* arg) {
    struct aqueue* queue = arg;
    for (size_t i = 0; i < 500; i++) {
        struct aqueue_node node = {.data = (void*)i};
        aqueue_push(queue, &node);
    }
    return NULL;
}

int main() {
    struct aqueue queue = {0};
    pthread_t thread;
    pthread_create(&thread, NULL, push, &queue);

    for (size_t i = 0; i < 500; i++) {
        struct aqueue_node node = {.data = (void*)i};
        aqueue_push(&queue, &node);
    }
    pthread_join(thread, NULL);
}
