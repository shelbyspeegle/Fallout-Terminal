
CC=gcc
CFLAGS=-Wall -O -I. -MMD
LFLAGS=-lncurses
PROGS=f3terminal
OBJECTS=

all: $(PROGS)

clean:
	/bin/rm -f $(PROGS) *.0 a.out *.d
	/bin/rm -fr docs doxygen.log *.dSYM

mclean: clean build
