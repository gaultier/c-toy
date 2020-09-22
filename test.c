#include <stdio.h>
#include <stdlib.h>

#include "aqueue.h"

int main() {
    struct aqueue queue = {0};
    int val = 1;
    struct aqueue_node node = {.data = &val};

    aqueue_push(&queue, &node);
}
