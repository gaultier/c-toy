#pragma once

#include <stddef.h>

typedef void (*free_fn)(void*);
typedef void* (*realloc_fn)(void*, size_t);
struct allocator {
    free_fn free;
    realloc_fn realloc;
};
