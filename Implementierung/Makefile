#compiler
CC = gcc

#FLAGS
CFLAGS = -g -Wall -Wextra -std=c11 -INaiveImplementation

all: main

main: NaiveImplementation/main.c NaiveImplementation/escapetime.c NaiveImplementation/bitmap.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

clean:
	rm -f main
	rm -f test.bmp
