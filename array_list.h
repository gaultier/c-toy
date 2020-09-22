#pragma once

#include <errno.h>

#include "allocator.h"
#include "utils.h"

struct array_list {
    void** data;
    size_t capacity;
    size_t len;
};

void array_list_init(struct array_list* array_list) {
    array_list->len = 0;
    array_list->capacity = 0;
    array_list->data = NULL;
}

void array_list_deinit(struct array_list* array_list,
                       struct allocator* allocator) {
    if (array_list->data != NULL) allocator->free(array_list->data);
}

int array_list_ensure_capacity(struct array_list* array_list, size_t capacity,
                               struct allocator* allocator) {
    PG_ASSERT_NOT_EQ(array_list, NULL, "%p");
    PG_ASSERT_NOT_EQ(allocator, NULL, "%p");

    array_list->data =
        allocator->realloc(array_list->data, capacity * sizeof(void*));
    if (array_list->data == NULL) return ENOMEM;

    array_list->capacity = capacity;

    return 0;
}

int array_list_append(struct array_list* array_list, void* item,
                      struct allocator* allocator) {
    PG_ASSERT_NOT_EQ(array_list, NULL, "%p");
    PG_ASSERT_NOT_EQ(item, NULL, "%p");

    if ((array_list->len + 1) >= array_list->capacity) {
        const size_t new_capacity = 1 + array_list->capacity * 2;

        int err;
        if ((err = array_list_ensure_capacity(array_list, new_capacity,
                                              allocator)) != 0)
            return err;
    }
    PG_ASSERT_NOT_EQ(array_list, NULL, "%p");
    PG_ASSERT_NOT_EQ(array_list->data, NULL, "%p");
    PG_ASSERT_COND(array_list->len, <, array_list->capacity, "%zu");
    PG_ASSERT_COND(array_list->capacity, >=, (size_t)1, "%zu");

    array_list->data[array_list->len] = item;
    array_list->len += 1;

    return 0;
}

void* array_list_get(struct array_list* array_list, size_t i) {
    PG_ASSERT_NOT_EQ(array_list, NULL, "%p");
    PG_ASSERT_NOT_EQ(array_list->data, NULL, "%p");

    void* item;
    buf_get_at(array_list->data, array_list->capacity, item, i);
    return item;
}

int array_list_set(struct array_list* array_list, size_t i, void* item) {
    PG_ASSERT_NOT_EQ(array_list, NULL, "%p");
    PG_ASSERT_NOT_EQ(array_list->data, NULL, "%p");

    if (i >= array_list->len) return EINVAL;

    array_list->data[i] = item;

    return 0;
}

int array_list_pop(struct array_list* array_list, void** item) {
    PG_ASSERT_NOT_EQ(array_list, NULL, "%p");
    PG_ASSERT_NOT_EQ(item, NULL, "%p");

    if (array_list->len == 0) return EINVAL;
    PG_ASSERT_NOT_EQ(array_list->data, NULL, "%p");

    buf_get_at(array_list->data, array_list->capacity, *item,
               array_list->len - 1);

    array_list->len -= 1;
    return 0;
}
