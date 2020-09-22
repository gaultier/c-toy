#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "aqueue.h"

int vals[AQUEUE_CAPACITY] = {0};

void* push(void* arg) {
    struct aqueue* queue = arg;
    for (size_t i = 500; i < AQUEUE_CAPACITY; i++) {
        vals[i] = i;
        printf("Auxiliary thread #%zu: pushing %zu len=%zu\n", i, i,
               aqueue_len(queue));
        aqueue_push(queue, &vals[i]);
        printf("Auxiliary thread #%zu: pushed=%zu len=%zu\n", i, i,
               aqueue_len(queue));

        int* val;
        while ((val = aqueue_pop(queue)) == NULL) {
            printf("Auxiliary thread #%zu: sleeping len=%zu\n", i,
                   aqueue_len(queue));
            pg_nanosleep(10);
            aqueue_push(queue, &vals[i]);
        }
        printf("Auxiliary thread #%zu: pop = %d len=%zu\n", i, *val,
               aqueue_len(queue));
    }
    return NULL;
}

int main() {
    struct aqueue queue = {0};
    pthread_t thread;
    pthread_create(&thread, NULL, push, &queue);

    for (size_t i = 0; i < 501; i++) {
        vals[i] = i;
        printf("Main thread: pushing %zu len=%zu\n", i, aqueue_len(&queue));
        aqueue_push(&queue, &vals[i]);
        printf("Main thread: pushed=%zu  len=%zu\n", i, aqueue_len(&queue));

        int* val;
        while ((val = aqueue_pop(&queue)) == NULL) {
            printf("Main thread #%zu: sleeping len=%zu\n", i,
                   aqueue_len(&queue));
            pg_nanosleep(10);
            aqueue_push(&queue, &vals[i]);
        }
        printf("Main thread: pop = %d len=%zu\n", *val, aqueue_len(&queue));
    }
    pthread_join(thread, NULL);
}
