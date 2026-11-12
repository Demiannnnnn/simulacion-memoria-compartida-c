CC      ?= gcc
CFLAGS  ?= -std=gnu11 -Wall -Wextra -O2
LDFLAGS ?= -pthread

.PHONY: all test clean

all: main

main: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

tests/test_main: tests/test_main.c main.c
	$(CC) $(CFLAGS) -o $@ tests/test_main.c $(LDFLAGS)

test: tests/test_main
	./tests/test_main

clean:
	rm -f main tests/test_main
