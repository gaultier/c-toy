#include "thread_safe_queue.h"

int main() {
    int data[5] = {1, 2, 3, 4, 5};
    int val = 0;
    buf_get_at(data, (size_t)5, val, (size_t)4);
    printf("%d\n", val);
}
