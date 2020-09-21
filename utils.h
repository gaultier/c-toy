#pragma once
#include <time.h>

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

#define PG_ASSERT_NOT_EQ(actual, expected, fmt)                       \
    do {                                                              \
        if (actual == expected) {                                     \
            fprintf(stderr,                                           \
                    __FILE__ ":%d:PG_ASSERT_EQ failed: expected=" fmt \
                             ", actual=" fmt,                         \
                    __LINE__, expected, actual);                      \
            exit(EINVAL);                                             \
        }                                                             \
    } while (0)

void pg_nanosleep(size_t ns) {
    const struct timespec time = {.tv_nsec = ns};
    nanosleep(&time, NULL);
}
