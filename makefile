CC=gcc 
CFLAGS=-Wall
all: reversi.c commands.c minmax.c
	$(CC) -o reversi reversi.c commands.c minmax.c $(CFLAGS)

clean:
	rm -f reversi
