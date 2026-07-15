#compiler
CC = gcc

#FLAGS
CFLAGS = -g -Wall -Wextra -std=c11 -INaiveImplementation -D_POSIX_C_SOURCE=200809L -O2

all: main

main: main.c NaiveImplementation/escapetime.c NaiveImplementation/tricorn_V1.c Simd/escapetime.c Simd/tricorn_V2.c \
	Circle/tricorn_V3.c Circle/escapetime.c LatencyThroughput/escapetime.c LatencyThroughput/tricorn_V4.c bitmap.c \
	bitmap.h testSuite.c testSuite.c testSuite.h MultiThreading/tricorn.c MultiThreading/tricorn.h colormap.h utils.h \
	helpMessage.c helpMessage.h MultiThreading/corecalculator.h graph.c graph.h
	$(CC) $(CFLAGS) -o $@ $^ -lm


clean:
	rm -f main
	rm -f test.bmp
