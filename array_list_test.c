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
