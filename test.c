#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "aqueue.h"

int* numbers;
const size_t len = 1000 * 1000 * 20;

void* thread_fn(void* arg) {
    struct aqueue* queue = arg;
    assert(arg != NULL);

    struct timeval start;
    gettimeofday(&start, NULL);

    for (size_t i = 0; i < 1000 * 1000; i++) {
        for (int j = 0; j < 6; j++) aqueue_push(queue, &numbers[i]);
        for (int j = 0; j < 6; j++) aqueue_pop(queue);
    }

    struct timeval end;
    gettimeofday(&end, NULL);

    size_t duration_us = end.tv_sec * 1000 * 1000 + end.tv_usec -
                         start.tv_sec * 1000 * 1000 + start.tv_usec;

    printf("Thread finished: duration: %zu us, avg op duration: %f us\n",
           duration_us, duration_us / (12.0 * 1000 * 1000));
    return NULL;
}

int main(int argc, char* argv[]) {
    assert(argc > 0);

    if (argc != 2) {
        printf("Usage: %s <num_cpus>\n", argv[0]);
        return 0;
    }

    struct aqueue_node* nodes = calloc(sizeof(struct aqueue_node), len);
    assert(nodes != NULL);

    struct aqueue queue;
    aqueue_init(&queue, nodes, len);

    numbers = malloc(sizeof(int) * len);
    assert(numbers != NULL);

    for (size_t i = 0; i < len; i++) numbers[i] = i;

    const size_t num_cpus = strtoll(argv[1], NULL, 10);
    pthread_t* threads = malloc(sizeof(pthread_t) * num_cpus);

    for (size_t i = 0; i < num_cpus; i++) {
        pthread_create(&threads[i], NULL, thread_fn, &queue);
    }

    for (size_t i = 0; i < num_cpus; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
