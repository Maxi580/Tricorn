#compiler
CC = gcc

#FLAGS
CFLAGS = -g -Wall -Wextra -std=c11 -INaiveImplementation

all: main usedsimd

main: NaiveImplementation/main.c NaiveImplementation/escapetime.c NaiveImplementation/bitmap.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

usedsimd: Simd/main.c Simd/escapetime.c Simd/bitmap.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

clean:
	rm -f main
	rm -f usedsimd
	rm -f test.bmp
