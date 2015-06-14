#ifndef __INTERFACE_H
#define __INTERFACE_H

#include <ncurses.h>
#include <unistd.h>

#include "common.h"

useconds_t TYPE_SPEED;
useconds_t PRINT_SPEED;
useconds_t TYPE_DELAY;

Point cur;

int rows, cols;
int terminalStartX;
int terminalStartY;

void initializeInterface();
void refreshBoard();
int terminalTooSmall();
void error_terminalTooSmall();
void mvtermprint( int y, int x, char *string, useconds_t speed );

void printScreen1();
void printScreen2();
void printScreen3();

void updateAttemptsLeft();
void printInputArea();
void printRegisters();

void highlight();
char *stringAtCursor();

int insideWord();
int insideHack();

void getUserInput();
void cursorUp();
void cursorDown();
void cursorLeft();
void cursorRight();

int calculateNextCurX( int keyDirection );

#endif /* __INTERFACE_H */