#compiler
CC = gcc

#FLAGS
CFLAGS = -g -Wall -Wextra -std=c11 -INaiveImplementation

all: main

main: main.c NaiveImplementation/escapetime.c NaiveImplementation/bitmap.c Simd/escapetime.c Simd/bitmap.c
	$(CC) $(CFLAGS) -o $@ $^ -lm


clean:
	rm -f main
	rm -f test.bmp
