#pragma once

#define PG_ASSERT_EQ(actual, expected, fmt)                           \
    do {                                                              \
        if (actual != expected) {                                     \
            fprintf(stderr,                                           \
                    __FILE__ ":%d:PG_ASSERT_EQ failed: expected=" fmt \
                             ", actual=" fmt,                         \
                    __LINE__, expected, actual);                      \
            exit(EINVAL);                                             \
        }                                                             \
    } while (0)
