CC = gcc
CFLAGS = -pthread -Wall -std=c99 -D_POSIX_C_SOURCE=199309L

all: keyTime.c lib.c
	$(CC) $(CFLAGS) keyTime.c lib.c -o keyTime

clean:
	rm keyTime *.o
