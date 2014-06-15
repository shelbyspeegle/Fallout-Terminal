
CC=gcc
CFLAGS=-Wall -g -O
LFLAGS=-lncurses
PROGS=hackterm

all: $(PROGS)

hackterm: hackterm.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

-include *.d

clean:
	/bin/rm -f $(PROGS) *.o a.out *.d
	/bin/rm -fr docs doxygen.log *.dSYM
