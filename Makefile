.POSIX:

.SUFFIX:

CFLAGS=-std=c99 -Wall -Wextra -g -I/usr/local/include -L/usr/local/lib


all: test actor

run_tests: test
	./aqueue_test
	./thread_pool_test

test: aqueue_test thread_pool_test

aqueue_test: aqueue_test.c aqueue.h utils.h
	$(CC) $(CFLAGS) -fsanitize=address -lcriterion $< -o $@

thread_pool_test: thread_pool_test.c thread_pool.h utils.h
	$(CC) $(CFLAGS) -fsanitize=address -lcriterion $< -o $@

actor: actor.c actor.h thread_pool.h aqueue.h utils.h
	$(CC) $(CFLAGS) -fsanitize=address $< -o $@

clean:
	rm -rf ./aqueue_test ./thread_pool_test ./a.out ./*.dSYM ./actor

.PHONY: test clean
