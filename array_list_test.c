#include "array_list.h"

#include <criterion/criterion.h>

Test(array_list, init_deinit) {
    struct array_list array_list;

    array_list_init(&array_list);
    cr_expect_eq(array_list.len, 0);
    cr_expect_eq(array_list.capacity, 0);
    cr_expect_eq(array_list.data, NULL);

    struct allocator allocator = {.realloc = realloc, .free = free};
    array_list_deinit(&array_list, &allocator);
}

Test(array_list, append) {
    struct array_list array_list;

    array_list_init(&array_list);

    struct allocator allocator = {.realloc = realloc, .free = free};

    int val = 1;
    cr_expect_eq(array_list_append(&array_list, &val, &allocator), 0);

    cr_expect_eq(array_list.len, 1);
    cr_expect_gt(array_list.capacity, 0);
    cr_expect_neq(array_list.data, NULL);
    cr_expect_eq(array_list.data[0], &val);
    cr_expect_eq(array_list_get(&array_list, 0), &val);

    array_list_deinit(&array_list, &allocator);
}

Test(array_list, pop) {
    struct array_list array_list;

    array_list_init(&array_list);

    struct allocator allocator = {.realloc = realloc, .free = free};

    int* other_val = NULL;
    cr_expect_eq(array_list_pop(&array_list, (void**)&other_val), EINVAL);
    cr_expect_eq(other_val, NULL);

    int val = 1;
    cr_expect_eq(array_list_append(&array_list, &val, &allocator), 0);

    cr_expect_eq(array_list_pop(&array_list, (void**)&other_val), 0);
    cr_expect_neq(other_val, NULL);
    cr_expect_eq(*other_val, 1);

    array_list_deinit(&array_list, &allocator);
}

Test(array_list, set) {
    struct array_list array_list;

    array_list_init(&array_list);

    struct allocator allocator = {.realloc = realloc, .free = free};

    int val = 1;
    cr_expect_eq(array_list_append(&array_list, &val, &allocator), 0);

    cr_expect_eq(array_list_get(&array_list, 0), &val);

    int new_val = 2;
    cr_expect_eq(array_list_set(&array_list, 1, &new_val), EINVAL);
    cr_expect_eq(array_list_set(&array_list, 0, &new_val), 0);
    cr_expect_eq(array_list_get(&array_list, 0), &new_val);

    array_list_deinit(&array_list, &allocator);
}

Test(array_list, add_one_assume_capacity) {
    struct array_list array_list;

    array_list_init(&array_list);

    struct allocator allocator = {.realloc = realloc, .free = free};

    cr_expect_eq(array_list_ensure_capacity(&array_list, 1, &allocator), 0);

    cr_expect_eq(array_list.len, 0);
    cr_expect_eq(array_list.capacity, 1);
    cr_expect_neq(array_list.data, NULL);

    int val = 1;
    array_list_add_one_assume_capacity(&array_list, &val);

    int* other_val = array_list_get(&array_list, 0);
    cr_expect_neq(other_val, NULL);
    cr_expect_eq(*other_val, 1);

    array_list_deinit(&array_list, &allocator);
}

