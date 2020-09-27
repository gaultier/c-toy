
#include "actor.h"

#include <criterion/criterion.h>

Test(actor, init_deinit) {
    struct allocator allocator = {.realloc = realloc, .free = free};

    struct actor_system actor_system;
    cr_expect_eq(actor_system_init(&actor_system, &allocator), 0);

    thread_pool_wait_until_finished(&actor_system.pool);
    actor_system_deinit(&actor_system);
}
