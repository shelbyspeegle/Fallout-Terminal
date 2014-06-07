
CC=gcc
CFLAGS=-Wall -g -O
LFLAGS=-lncurses
PROGS=f3terminal hackterm

all: $(PROGS)

f3terminal: f3terminal.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

hackterm: hackterm.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

-include *.d

clean:
	/bin/rm -f $(PROGS) *.o a.out *.d
	/bin/rm -fr docs doxygen.log *.dSYM
