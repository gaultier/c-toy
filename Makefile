.POSIX:

.SUFFIX:

CFLAGS=-std=c99 -Wall -Wextra -g  

thread_safe_queue_test: thread_safe_queue_test.c thread_safe_queue.h
	$(CC) $(CFLAGS) -fsanitize=address -lcriterion $< -o $@

thread_pool_test: thread_pool_test.c thread_pool.h
	$(CC) $(CFLAGS) -fsanitize=address -lcriterion $< -o $@
