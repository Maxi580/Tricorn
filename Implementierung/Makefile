#compiler
CC = gcc

#FLAGS
CFLAGS = -g -Wall -Wextra -std=c11 -INaiveImplementation -D_POSIX_C_SOURCE=200809L -O2

all: main

main: main.c NaiveImplementation/escapetime.c NaiveImplementation/image.c Simd/escapetime.c Simd/image.c \
	Sym/escapetime.c Sym/image.c Circle/image.c Circle/escapetime.c LatencyThroughput/escapetime.c \
	LatencyThroughput/image.c bitmap.c bitmap.h testSuite.c correctnesTests.c correctnesTests.h \
	MultiThreading/image.c MultiThreading/image.h
	$(CC) $(CFLAGS) -o $@ $^ -lm


clean:
	rm -f main
	rm -f test.bmp
