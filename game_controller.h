#ifndef __GAMECONTROLLER_H
#define __GAMECONTROLLER_H

#include <stdio.h>
#include "password.h"

FILE *fr;

void run();

int getDebugStatus();
void setDebugStatus( int newDebugStatus );

int getHardmodeStatus();

int getLoggedInStatus();

void setAttemptsLeft( int newAttemptsLeft );
int getAttemptsLeft();

char * getBoard();

char ** getMessages();

PasswordPtr * getHacks();
PasswordPtr * getPasswords();

int getNumHacks();
int getNumPasswords();
int getPasswordLength();
int getMaxNumMessages();

void accessSystem();
void tryPassword();

void exitUos( int exitStatus );

void loadFiles();

#endif /* __GAMECONTROLLER_H */