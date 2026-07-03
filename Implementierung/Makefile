#compiler
CC = gcc

#FLAGS
CFLAGS = -g -Wall -Wextra -std=c11 -INaiveImplementation -D_POSIX_C_SOURCE=200809L -O2

all: main

main: main.c NaiveImplementation/escapetime.c NaiveImplementation/bitmap.c Simd/escapetime.c Simd/bitmap.c Sym/escapetime.c Sym/bitmap.c Circle/bitmap.c Circle/escapetime.c
	$(CC) $(CFLAGS) -o $@ $^ -lm


clean:
	rm -f main
	rm -f test.bmp
