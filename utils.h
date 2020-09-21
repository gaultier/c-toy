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

#define buf_get_at(data, len, val, index)                                   \
    do {                                                                    \
        PG_ASSERT_NOT_EQ(data, NULL, "%p");                                 \
        if (index >= len) {                                                 \
            fprintf(                                                        \
                stderr,                                                     \
                __FILE__                                                    \
                ":%d:Error: accessing array at index %zu but len is %zu\n", \
                __LINE__, index, len);                                      \
            exit(EINVAL);                                                   \
        }                                                                   \
        val = data[index];                                                  \
    } while (0)

#define buf_set_at(data, len, val, index)                                   \
    do {                                                                    \
        PG_ASSERT_NOT_EQ(data, NULL, "%p");                                 \
        if (index >= len) {                                                 \
            fprintf(                                                        \
                stderr,                                                     \
                __FILE__                                                    \
                ":%d:Error: accessing array at index %zu but len is %zu\n", \
                __LINE__, index, len);                                      \
            exit(EINVAL);                                                   \
        }                                                                   \
        data[index] = val;                                                  \
    } while (0)

