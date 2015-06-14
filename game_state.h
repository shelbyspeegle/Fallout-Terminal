#ifndef __GAMESTATE_H
#define __GAMESTATE_H

#include "utilities.h"
#include "password.h"
#include "common.h"

#define MAX_MESSAGES 15
#define MAX_MESSAGE_LENGTH 13
#define NUM_PASSWORDS 10  /* TODO: This should be a variable */
#define NUM_HACKS 7  /* TODO: Find the real number */

boolean HARDMODE;
int PASSWORD_LENGTH;

boolean debug;
boolean loggedIn;

char **messages;  /* Array of message strings. */
char board[408];  /* Characters that make up the game board. */
char **wordList;
PasswordPtr *hacks;
PasswordPtr *passwords;
int correct;  /* The index position of the correct password in passLocations[] */
int passwordsLeftOnBoard;

int attemptsLeft;

void pushMessage( const char *newMessage );
void genHacks();
void genPasswords();
char *uniqueRandomWord();
int numberOfCorrectChars( const char *checkWord );
void setup();
void lockTerminal();
void refreshPasswords();
void loadFiles();

#endif /* __GAMESTATE_H */