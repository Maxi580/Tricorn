#compiler
CC = gcc

#FLAGS
CFLAGS = -g -Wall -Wextra -std=c11 -INaiveImplementation -D_POSIX_C_SOURCE=200809L -O2

all: main

main: main.c NaiveImplementation/escapetime.c NaiveImplementation/tricornV0.c Simd/escapetime.c Simd/tricornV1.c \
	Sym/escapetime.c Sym/tricornV2.c Circle/tricornV3.c Circle/escapetime.c LatencyThroughput/escapetime.c \
	LatencyThroughput/tricornV4.c bitmap.c bitmap.h testSuite.c correctnesTests.c correctnesTests.h \
	MultiThreading/tricorn.c MultiThreading/tricorn.h colormap.h colormap.c helpMessage.c helpMessage.h
	$(CC) $(CFLAGS) -o $@ $^ -lm


clean:
	rm -f main
	rm -f test.bmp
