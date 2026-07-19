#compiler
CC = gcc

#FLAGS
CFLAGS = -g -Wall -Wextra -std=c17 -INaiveImplementation -D_POSIX_C_SOURCE=200809L -O2 -pthread

SOURCES = main.c \
		  NaiveImplementation/escapetime.c \
		  NaiveImplementation/tricorn_V1.c \
		  Simd/escapetime.c \
		  Simd/tricorn_V2.c \
		  Circle/tricorn_V3.c \
		  Circle/escapetime.c \
		  LatencyThroughput/escapetime.c \
		  LatencyThroughput/tricorn_V4.c \
		  MultiThreading/tricorn.c \
		  bitmap.c \
		  testSuite.c \
		  helpMessage.c \
		  graph.c

all: main

main: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ -lm


clean:
	rm -f main
	rm -f test.bmp

.PHONY: all clean