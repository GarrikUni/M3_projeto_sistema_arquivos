CC=gcc
CFLAGS=-Wall -g

OBJS=main.o filesystem.o btree.o

all: fs

fs: $(OBJS)
	$(CC) -o fs $(OBJS)

clean:
	rm -f *.o fs
