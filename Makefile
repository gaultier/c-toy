.POSIX:

.SUFFIX:

CFLAGS=-std=c99 -Wall -Wextra -g -I/usr/local/include -L/usr/local/lib


all: test actor

run_tests: test
	./thread_safe_queue_test
	./thread_pool_test

test: thread_safe_queue_test thread_pool_test array_list_test

thread_safe_queue_test: thread_safe_queue_test.c thread_safe_queue.h utils.h
	$(CC) $(CFLAGS) -fsanitize=address -lcriterion $< -o $@

thread_pool_test: thread_pool_test.c thread_pool.h utils.h array_list.h
	$(CC) $(CFLAGS) -fsanitize=address -lcriterion $< -o $@

array_list_test: array_list_test.c array_list.h utils.h array_list.h
	$(CC) $(CFLAGS) -fsanitize=address -lcriterion $< -o $@

actor: actor.c actor.h thread_pool.h thread_safe_queue.h utils.h array_list.h
	$(CC) $(CFLAGS) -fsanitize=thread $< -o $@

clean:
	rm -rf ./thread_safe_queue_test ./thread_pool_test ./a.out ./*.dSYM ./actor

.PHONY: test clean
