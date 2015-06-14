CC=gcc
CFLAGS=-Wall -g -O
LFLAGS=-lncurses
PROGS=fallout-terminal
DEPENDENCIES=game_controller.o interface.o game_state.o password.o utilities.o

all: $(PROGS)

fallout-terminal: main.o $(DEPENDENCIES)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

-include *.d

clean:
	/bin/rm -f $(PROGS) *.o *.d *.dSYM