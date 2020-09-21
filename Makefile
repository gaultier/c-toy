.POSIX:

.SUFFIX:

CFLAGS=-std=c99 -Wall -Wextra -g  

test: thread_safe_queue_test thread_pool_test
	./thread_safe_queue_test
	./thread_pool_test

thread_safe_queue_test: thread_safe_queue_test.c thread_safe_queue.h
	$(CC) $(CFLAGS) -fsanitize=address -lcriterion $< -o $@

thread_pool_test: thread_pool_test.c thread_pool.h
	$(CC) $(CFLAGS) -fsanitize=address -lcriterion $< -o $@

actor: actor.c
	$(CC) $(CFLAGS) -fsanitize=address $< -o $@

clean:
	rm -rf ./thread_safe_queue_test ./thread_pool_test ./a.out ./*.dSYM ./actor

.PHONY: test clean
