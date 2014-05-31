
CC=gcc
CFLAGS=-Wall -g -O
LFLAGS=-lncurses
PROGS=f3terminal hello

all: $(PROGS)

f3terminal: f3terminal.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

hello: hello.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

-include *.d

clean:
	/bin/rm -f $(PROGS) *.o a.out *.d
	/bin/rm -fr docs doxygen.log *.dSYM
