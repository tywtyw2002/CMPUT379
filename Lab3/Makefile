CC=gcc
CFLAGS=-c -Wall 
LD=-lpthread -lncurses


all: saucer

saucer: saucer.o ascii.o
	$(CC)  -o saucer $^ $(LD)

%.o: %.c 
	$(CC) $(CFLAGS) $(LD)  -c -o $@ $<


clean:
	rm -rf *.o saucer
