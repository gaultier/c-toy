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
