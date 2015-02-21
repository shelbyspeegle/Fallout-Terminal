/*
 * hackterm.c
 *
 *  Created on: May 3, 2014
 *      Author: shelbyspeegle
 */

#include <ncurses.h>  /* ncurses.h includes stdio.h */
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>  /* for srand(time(NULL)) */

#include "common.h"  /* Include common data types */
#include "password.h"
#include "utilities.h"

#define MAX_MESSAGES 15
#define MAX_MESSAGE_LENGTH 13
#define NUM_PASSWORDS 10  /* TODO: This should be a variable */
#define NUM_HACKS 7  /* TODO: Find the real number */

char *registers[] = {
    "0xF964", "0xF970", "0xF97C", "0xF988", "0xF994", "0xF9A0", "0xF9AC",
    "0xF9B8", "0xF9C4", "0xF9D0", "0xF9DC", "0xF9E8", "0xF9F4", "0xFA00",
    "0xFA0C", "0xFA18", "0xFA24", "0xFA30", "0xFA3C", "0xFA48", "0xFA54",
    "0xFA60", "0xFA6C", "0xFA78", "0xFA84", "0xFA90", "0xFA9C", "0xFAA8",
    "0xFA84", "0xFAC0", "0xFACC", "0xFAD8", "0xFAE4", "0xFAF0"
};
char **messages;  /* Array of message strings. */
char board[408];  /* Characters that make up the game board. */
char **wordList;
int passwordLength;
PasswordPtr *hacks;
PasswordPtr *passwords;
int rows, cols;
int trysLeft = 4;
int correct;  /* The index position of the correct password in passLocations[] */
Point cur;
int TYPE_SPEED = 24000;  /* TODO: make these constants when program is finished */
int PRINT_SPEED = 18000;
boolean debug;
boolean hardmode = FALSE;
int passwordsLeftOnBoard = NUM_PASSWORDS;
FILE *fr;

void setup();
void printInputArea();
void refreshBoard();
void pushMessage(const char *newMessage);
boolean tryPassword();
void genPasswords();
void genHacks();
int insideWord();
int insideHack();
void highlight();
char *stringAtCursor();
int numberOfCorrectChars(const char *checkWord);
void accessSystem();
void exitUos();
void lockTerminal();
void manualInputMode();
void autoInputMode();
int calculateNextCurX(int keyDirection);
void refreshPasswords();
char *uniqueRandomWord();

void mvtermprint( int y, int x, char *string, int speed );

int main( int argc, char **argv ) {

  debug = (argc == 2 && argv[1][0] == 'd');

  srand( (unsigned int)time(0) );  /* Seed rand with this so it is more random. */

  initscr();  /* Start ncurses mode. */
  getmaxyx( stdscr, rows, cols );  /* Capture the terminal window size. */
  keypad( stdscr, TRUE );  /* Converts arrow key input to usable chars. */

  if (rows < 24 || cols < 55) {  /* Check to see if window is big enough. */
    /*TODO: Make terminal centered at all resolutions */

    endwin();
    printf("ERROR: Terminal window is too small,\n");
    printf("       minimum size of 24x55 to run.\n");

    return EXIT_ERROR;
  }

  passwordLength = 8;

  wordList = malloc( sizeof(char *) * 40 );

  /* Word lengths between 4-15 */

  char *fileName = (char *) malloc( sizeof(char) * 8 );

  sprintf(fileName, "%i.txt", passwordLength);

  fr = fopen(fileName, "r");

  char *line = NULL;
  size_t lineCapp = 100;

  int m = 0;

  for ( m=0; m<40; m++ ) {
    getline( &line, &lineCapp, fr );
    wordList[m] = malloc( sizeof(char) * passwordLength );
    strncpy( wordList[m], line, passwordLength );
  }

  free(line);
  fclose(fr);

  /* Cut to the chase if debug mode is on. */
  if ( debug ) TYPE_SPEED = PRINT_SPEED = 0;

  /* Start of display. */
  mvtermprint( 1, 1, "SECURITY RESET...", PRINT_SPEED);
  clear();
  mvtermprint( 3, 1, "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK", PRINT_SPEED);

  hardmode ? manualInputMode() : autoInputMode();

  setup();

  boolean loggedIn = FALSE;

  while (1) {  /* Start the game. */

    /* Update the attempts left section. */
    mvprintw( 4, 21, "        ");  /* Clear the tries. */

    if (trysLeft == 1) {
      attron(A_BLINK);  /* eh blinkin'! */
      mvprintw(2, 1, "!!! WARNING: LOCKOUT IMMINENT !!!");
      attroff(A_BLINK);
    } else {
      mvprintw(2, 1, "ENTER PASSWORD NOW               ");
    }

    /* Place the symbols for number of tries left. */
    attron(A_STANDOUT);
    switch (trysLeft) {
      case 4:
        mvprintw( 4, 27, " ");  /* 4th */
      case 3:
        mvprintw( 4, 25, " ");  /* 3rd */
      case 2:
        mvprintw( 4, 23, " ");  /* 2nd */
      case 1:  /* Lockout imminent. */
        mvprintw( 4, 21, " ");  /* 1st */
        attroff(A_STANDOUT);
        break;
      default:  /* Game over. */
        attroff(A_STANDOUT);
        lockTerminal();
    }

    mvprintw(4, 1, "%i", trysLeft);

    refreshBoard();

    move(cur.y, cur.x);

    highlight();
    refresh();

    printInputArea();

    if (loggedIn) {
      usleep(1000000*3);
      accessSystem();
    }

    int uInput = getch();  /* Get user input from keyboard (pause) */
    usleep(10);  /* Reduces cursor jump if arrows are held down */

    switch (uInput) {
      case '\n' :  /* TODO: test Linux */
        loggedIn = tryPassword();
        break;
      case KEY_UP :
        if (cur.y != 6)
          cur.y--;
        break;
      case KEY_DOWN :
        if (cur.y != 22)
          cur.y++;
        break;
      case KEY_LEFT :
        if (insideWord() > -1 )                /* If the cursor is inside a word-                            */
          cur.x = calculateNextCurX(uInput);  /*   figure out the next cur.x location.                      */
        else if (cur.x == 28)                   /* Else if the cursor is at the left bound of the right side- */
          cur.x = 19;                           /*   move the cursor to the right bound of the left side.     */
        else if (cur.x != 8)                    /* As long as the cursor is not at the far left of the board- */
          cur.x--;                              /*   move the cursor left.                                    */
        break;
      case KEY_RIGHT :
        if (insideWord() > -1 )                /* If the cursor is inside a word-                             */
          cur.x = calculateNextCurX(uInput);  /*   figure out the next cur.x location.                       */
        else if (cur.x == 19)                   /* Else if the cursor is at the right bound of the left side-  */
          cur.x = 28;                           /*   move the cursor to the left bound of the right side.      */
        else if (cur.x != 39)                   /* As long as the cursor is not at the far right of the board- */
          cur.x++;                              /*   move the cursor right.                                    */
        break;
      case '-' :
        trysLeft--;
        break;
      case '+' :
        if (trysLeft < 4) {
          trysLeft++;
          pushMessage("Allowance");
          pushMessage("replenished.");
        }
        break;
      case 'a' :
        loggedIn = TRUE;
        break;
      case 'q' :  /* Quit key */
        exitUos();
        exit( EXIT_SUCCESS );
      default:
        break;
    }
  }
}

void setup() {
  clear();
  noecho();  /* Silence user input */

  /* Board Set-up ////////////////////////////////////////////////////////////*/
  mvprintw( 22, 41, ">" );
  refresh();

  /* Place cursor at starting position */
  cur.y = START_Y;
  cur.x = START_X;

  messages = malloc( sizeof(char*) * MAX_MESSAGES);
  hacks = malloc( sizeof(char*) * NUM_HACKS);
  passwords = malloc( sizeof(PasswordPtr *) * NUM_PASSWORDS );
  mvtermprint( 1, 1, "ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL", PRINT_SPEED );
  mvtermprint( 2, 1, "ENTER PASSWORD NOW", PRINT_SPEED );
  mvtermprint( 4, 1, "4 ATTEMPT(S) LEFT : ", PRINT_SPEED );

  /* Populate Board Array with Trash /////////////////////////////////////////*/
  int i;
  for (i = 0; i < 408; i++) {
    board[i] = genTrash();
  }

  genPasswords();
  refreshPasswords();
  genHacks();

  /* TODO: clean up the code below, it is pretty messy */
  /* Print Registers /////////////////////////////////////////////////////////*/
  int printX = 1;
  int printY = 6;

  /* Print left half of game */
  for (i = 0; i < 17; i++) {
    mvtermprint(printY, printX, registers[i], PRINT_SPEED/6 );  /* Print registers */
    printX+=7;
    refresh();

    int j;
    for (j = 0; j < 12; j++) {  /* iterate through each char in array */
      mvprintw(printY, printX++, "%c", board[ j + (12 * i) ]);
      usleep( (useconds_t) PRINT_SPEED/6 );
      refresh();
    }

    printX = 1;
    printY++;
  }

  printY = 6;
  printX = 21;

  /* Print right half of game */
  for (i = 0; i < 17; i++) {
    mvtermprint(printY, printX, registers[i+17], PRINT_SPEED/6 );  /* Print registers */
    printX+=7;
    refresh();

    int j;
    for (j = 0; j < 12; j++) {  /* iterate through each char in array */
      mvprintw(printY, printX++, "%c", board[204 + j + (12 * i) ]);
      usleep( (useconds_t) PRINT_SPEED/6 );
      refresh();
    }

    printX = 21;
    printY++;
  }
}

void printInputArea() {
  int i;
  for (i=0; i < MAX_MESSAGES; i++) {
    if (messages[i])
      mvprintw(20-i, 41, "%s", messages[i]);  /* Print messages that exist */
  }

  mvprintw( 22, 42, "              ");  /* Clear input line */

  mvtermprint( 22, 42, stringAtCursor(), PRINT_SPEED );
}

void refreshBoard() {
  int i;
  for (i = 0; i < 408; i++) {  /* iterate through each char in array */
    mvprintw(arrayToPoint(i).y, arrayToPoint(i).x, "%c", board[i]);
  }
}

void pushMessage(const char *newMessage) {
  char *fullMsg = malloc(sizeof(char) * MAX_MESSAGE_LENGTH + 1);

  /* Start the message off with a '>' */
  fullMsg[0] = '>';

  /* Copy all of the contents of the passed in string. */
  int k;
  for (k=1; k<strlen(newMessage)+1; k++) {
    fullMsg[k] = newMessage[k-1];
  }

  /* Fill the rest of the string with spaces. */
  while (k < MAX_MESSAGE_LENGTH) {
    fullMsg[k++] = ' ';
  }

  int i;
  for (i=0; i < MAX_MESSAGES; i++) {
    messages[MAX_MESSAGES-i] = messages[MAX_MESSAGES-i-1];
  }
  messages[0] = fullMsg;
}

boolean tryPassword() {
  if (insideWord() >= 0) {  /* Test if the cursor on a Password. */
    int k = insideWord();  /* Set k to the start of the word the cursor is in. */
    char *check = malloc(sizeof(char) * passwordLength);
    int j;
    for (j=0; j<passwordLength; j++) {
      check[j] = board[k++];
    };

    pushMessage(stringAtCursor());

    int i = numberOfCorrectChars(check);
    if (i == passwordLength) {
      pushMessage("Exact match!");
      pushMessage("Please wait");
      pushMessage("while system");
      pushMessage("is accessed.");

      return TRUE;
    } else {
      pushMessage("Entry denied");
      char *stringBuild = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
      sprintf(stringBuild, "%i/%i correct.", i, passwordLength);
      pushMessage(stringBuild);
      trysLeft--;
    }
  } else if (insideHack() >= 0) {  /* Test if the cursor on a Hack. */

    int a = insideHack();

    if (passwordsLeftOnBoard > 1) {  /* More than one Password on the board. */

      int i;
      for (i = 0; i < NUM_HACKS; i++) {  /* Iterate through all words. */
        if (a == hacks[i]->position ) {
          pushMessage(hacks[i]->content);
          hacks[i]->position = -1;  /* Make this hack unusable. */
        }
      }

      pushMessage("Dud removed.");

      boolean flag = FALSE;

      /* TODO maybe try a do-while here. */
      while ( !flag && passwordsLeftOnBoard > 1 ) {  /* Search for incorrect password to remove as a dud. */
        int min = 0;
        int max = NUM_PASSWORDS-1;

        int j = ( rand() % (max+1-min) ) + min;  /* Pick a random incorrect Password. */

        if (!passwords[j]->correct && !passwords[j]->removed) {

          /* Copy over contents of incorrect password with dots. */
          char *pointer = passwords[j]->content;
          passwords[j]->removed = TRUE;
          passwordsLeftOnBoard--;

          int k = 0;
          while (pointer[k] != '\0') {
            pointer[k] = '.';
            k++;
          }

          flag = TRUE;
        }
      }

      refreshPasswords();

      /* TODO provide logic for allowance replenishing */

    } else {  /* Only one password is left on the board. */

      /* TODO refactor this. It is a serious waste. */

      int i;
      for (i = 0; i < NUM_HACKS; i++) { /* iterate through all words */
        if (a == hacks[i]->position ) {
          pushMessage(hacks[i]->content);
        }
      }

      pushMessage("Entry denied");
    }
  } else { /* The cursor is on Trash. */
    pushMessage(stringAtCursor());
    pushMessage("Entry denied");

    char *stringbuild = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
    sprintf(stringbuild, "0/%i correct.", passwordLength);
    pushMessage(stringbuild);
    trysLeft--;
  }

  return FALSE;
}

void genHacks() {
  /* TODO: Add this logic... If hack is placed inside another, set the contents of the hack we are inside to the hack that is inside. */

  int newHackPositions[NUM_HACKS];
  int hackRow, hackRowPosition, hackStart, hackEnd, passwordStart, passwordEnd;

  int i;
  for ( i=0; i<NUM_HACKS; i++ ) {

    hacks[i] = createHack();

    boolean validArrayPosition = FALSE;

    while ( !validArrayPosition) {

      validArrayPosition = TRUE; /* TODO: fix... TOO HACKY! */

      /* Pick a row from 0 - 33 */
      hackRow = rand() % 34;

      /* Pick a spot in the row so that the hack remains on a single line. */
      hackRowPosition = rand() % ( 12  - hacks[i]->size ) ;

      hackStart = ( hackRow *12 ) + hackRowPosition;
      hackEnd = hackStart + hacks[i]->size;

      int j;
      /* Make sure the hack does not collide with any passwords. */
      for ( j=0; j<NUM_PASSWORDS; j++ ) {
        passwordStart = passwords[j]->position;
        passwordEnd = passwordStart + passwords[j]->size;

        validArrayPosition = validArrayPosition && ( hackEnd < passwordStart || hackStart > passwordEnd);

        if ( !validArrayPosition) {
          break; /* We do not need to look any further. */
        }
      }

      /* If hack collides with other hack, make sure that it is inside. */
      for ( j=0; j<i; j++ ) {
        /* Position is not valid if our hack starts at the same spot as another. */
        if ( hackStart == hacks[j]->position ) {
          validArrayPosition = FALSE;
          break;
        }

        /* Hacks cannot overlap. */
        if ( hackStart < hacks[j]->position && hackEnd < hacks[j]->position + hacks[j]->size && hackEnd > hacks[j]->position ) {
          validArrayPosition = FALSE;
          break;
        }

        /* Hack can be placed inside another hack. */
        if ( hackStart > hacks[j]->position && hackStart < hacks[j]->position + hacks[j]->size ) {
          boolean useSameBracket = sameBracketType( hacks[i], hacks[j] );
          boolean completelyInside = hackEnd < hacks[j]->position + hacks[j]->size;
          boolean shareClosingBracket = hackEnd == hacks[j]->position + hacks[j]->size;

          /* As long as it is nested completely inside and the hacks don't use the same brackets. */
          /* Or if hacks use the same closing bracket they may share that. */
          if ( (completelyInside && !useSameBracket) || (shareClosingBracket && useSameBracket) ) {
            validArrayPosition = validArrayPosition && TRUE;
          } else {
            validArrayPosition = FALSE;
            break;
          }
        }
      }
    }

    /* Safe spot for hack successfully found! */
    newHackPositions[i] = hackStart;
  }

  /* Set the locations of the newly created hacks. */
  for ( i=0; i<NUM_HACKS; i++ ) {
    setHackPosition( hacks[i], newHackPositions[i] );
  }

  int currLocation = 0;
  for ( i = 0; i < NUM_HACKS; i++ ) {
    currLocation = hacks[i]->position;

    char *s = hacks[i]->content;
    char c;
    int j = 0;
    while ( (c = s[j++]) ) {
      board[currLocation++] = c;
    }
  }
}

void genPasswords() {  /* Fill the passwords array with Passwords */

  int max = 407-1-passwordLength;
  int min = 0;
  int newPasswordPositions[NUM_PASSWORDS];

  int i;
  for ( i=0; i<NUM_PASSWORDS; i++ ) {
    boolean notValidPosition = TRUE;
    int positionForNewPassword;
    while (notValidPosition) {
      positionForNewPassword = ( rand() % (max+1-min) ) + min;  /* Pick a spot on the board */

      /* TODO: This is hacky. Setting flag to FALSE until something below trips it to TRUE */
      notValidPosition = FALSE;

      int j;
      for ( j=0; j<i; j++ ) {  /* Make sure current position is clear of every word generated */
        if ( positionForNewPassword < newPasswordPositions[j] + passwordLength + 1
            && positionForNewPassword +passwordLength > newPasswordPositions[j]) {
          notValidPosition = TRUE;
        }
      }
    }

    /* Place password at positionForNewPassword */
    newPasswordPositions[i] = positionForNewPassword;
  }

  int j;
  for ( j=0; j<NUM_PASSWORDS; j++ ) {
    passwords[j] = createPassword(uniqueRandomWord(), newPasswordPositions[j] );
  }

  /* Pick one of the passwords on the board to be correct. */
  max = NUM_PASSWORDS-1;
  min = 0;
  correct = ( rand() % (max+1-min) ) + min;  /* 0, NUM_PASSWORDS-1 */
  passwords[correct]->correct = TRUE;
}

int insideWord() {  /* if inside word, return array start position, else -1 */
  int a = arrayFromYX(cur.y, cur.x);

  int i;
  for (i = 0; i < NUM_PASSWORDS; i++) {  /* iterate through all words */
    if (a >= passwords[i]->position && a < passwords[i]->position + passwordLength ) {
      return passwords[i]->position;
    }
  }

  return -1;
}

int insideHack() {  /* if inside word, return array start position, else -1 */
  int a = arrayFromYX(cur.y, cur.x);

  int i;
  for (i = 0; i < NUM_HACKS; i++) {  /* iterate through all words */
    if (a == hacks[i]->position )
      return hacks[i]->position;
  }

  return -1;
}

void highlight() {
  attron(A_STANDOUT);

  int a = arrayFromYX(cur.y, cur.x);

  if (insideWord() >= 0) {

    /* TODO Do removed passwords highlight? */

    a = insideWord();
    int i;
    for (i = 0; i < NUM_PASSWORDS; i++) {  /* iterate through all words */
      if (a == passwords[i]->position ) {
        for ( i=0; i<passwords[i]->size; i++) {
          mvprintw(arrayToPoint(a).y, arrayToPoint(a).x, "%c", board[a]);
          a++;
        }
        break;
      }
    }

    attroff(A_STANDOUT);
    return;
  } else if (insideHack() >= 0) {
    a = insideHack();

    int i;
    for (i = 0; i < NUM_HACKS; i++) {  /* iterate through all words */
      if (a == hacks[i]->position ) {
        int j;
        for ( j=0; j<hacks[i]->size; j++) {
          mvprintw(arrayToPoint(a).y, arrayToPoint(a).x, "%c", board[a]);
          a++;
        }
      }
    }

    attroff(A_STANDOUT);
    return;
  }

  printw("%c", board[a]);
  attroff(A_STANDOUT);
}

char *stringAtCursor() {  /* TODO: Try refactoring this. */
  if (insideWord() >= 0) {  /* Cursor is in word. */
    char *check = malloc(sizeof(char) * passwordLength);

    int k = insideWord();

    int i;
    for ( i=0; i<passwordLength; i++ ) {
      check[i] = board[k++];
    };

    return check;
  } else if (insideHack() >= 0) {

    /* Find the hack that we are inside. */
    int hackPosition = insideHack();

    int i;
    for ( i=0; i< NUM_HACKS; i++) {
      if (hacks[i]->position == hackPosition) {
        return hacks[i]->content;
      }
    }
  }

  /* Cursor is on a single char. */
  char *returner = malloc(sizeof(char) * 2);
  returner[0] = board[arrayFromYX(cur.y, cur.x)];
  returner[1] = '\0';
  return returner;
}

void mvtermprint( int y, int x, char *string, int speed ) {
  /*
      TODO: Debug - why does strlen return 14?
      if (len == 14) {
          mvprintw(0,0, "FAILURE");
          getch();
      }
      mvprintw(0,0, "%i ", len);
  */

  int i;
  int len = (int) strlen(string);

  if (speed == TYPE_SPEED && speed > 0) {
    mvprintw( y, x++, ">" );
    refresh();

    usleep(1000000);
  }

  if (len == 1) {
    mvprintw( y, x++, "%c", string[0]);
    refresh();
  } else {
    for (i=0; i < len; i++) {
      mvprintw( y, x++, "%c", string[i]);
      refresh();
      usleep( (useconds_t) speed );
    }
  }

  if (speed == TYPE_SPEED && speed > 0) {
    usleep(1000000);
  }
}

int numberOfCorrectChars(const char *checkWord) {
  if (strlen(checkWord) != passwordLength) {
    /* TODO: properly free memory and end the program */
    endwin();
    fprintf(stderr, "ERROR: numberOfCorrectChars called on string of length %i when passwords are of length %i.\n",
        (int) strlen(checkWord), passwordLength);
    exit(1);
  }

  int count = 0;

  int i;
  for (i=0; i < passwordLength; i++) {
    if (passwords[correct]->content[i] == checkWord[i]) {
      count++;
    }
  }

  return count;
}

void accessSystem() {
  int fastPrintSpeed = PRINT_SPEED/3;

  clear();

  mvtermprint( 1, 1, "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK", fastPrintSpeed);
  mvtermprint( 3, 1, " LOGIN ADMIN", fastPrintSpeed);
  mvtermprint( 5, 1, "ENTER PASSWORD NOW", fastPrintSpeed);

  mvprintw( 7, 1, ">");
  refresh();
  usleep(1000000);  /* TODO: get exact time */
  int i;
  for (i=0; i < passwordLength; i++) {
    mvprintw(7, 3+i, "*");
    usleep( (useconds_t) TYPE_SPEED*3);
    refresh();
  }
  usleep(1000000);  /* TODO: get exact time */

  clear();

  mvtermprint( 1, 7, "ROBCO INDUSTRIES UNIFIED OPERATING SYSTEM", fastPrintSpeed);
  mvtermprint( 2, 9, "COPYRIGHT 2075-2077 ROBCO INDUSTRIES", fastPrintSpeed);
  mvtermprint( 3, 22, "-Server 1-", fastPrintSpeed);

  /* If Lock ///////////////////////////////////////////////////////////////*/
  mvtermprint( 5, 9, "SoftLock Solutions, Inc", fastPrintSpeed);
  mvtermprint( 6, 5, "\"Your Security is Our Security\"", fastPrintSpeed);
  mvtermprint( 7, 5, ">\\ Welcome, USER", fastPrintSpeed);
  mvtermprint( 8, 5, "_______________________________", fastPrintSpeed);

  int selection = 0;

  int numOptions = 2;
  char *menu[] = {
      "Disengage Lock",
      "Exit"
  };

  /* Menu */
  while (1) {
    /* Print Menu */
    int j;
    for (j=0; j < numOptions; j++) {
      if (selection == j) {
        attron(A_STANDOUT);
        mvprintw( 9+j, 5, "                                            ");
        mvprintw( 9+j, 5, "> %s", menu[j]);
        attroff(A_STANDOUT);
      } else {
        mvprintw( 9+j, 5, "                                            ");
        mvprintw(9+j, 5, "> %s", menu[j]);
      }
    }

    mvprintw( 22, 5, "                                        ");
    mvprintw( 22, 5, "> ");

    int uInput = getch();  /* Get user input from keyboard (pause). */
    usleep(10);  /* Reduces cursor jump if arrows are held down. */

    switch (uInput) {
      case '\n' :           /* TODO: test Linux */
        mvtermprint(22, 7, menu[selection], PRINT_SPEED );
        if (getch() == '\n') {
          mvprintw( 22, 5, "                                        ");
          mvprintw( 22, 5, "> ");
          mvtermprint( 22, 7, "Exiting...", PRINT_SPEED);
          usleep(1000000*3);
          exitUos();
        }
        break;
      case KEY_UP :
        selection == 0 ? selection = 1 : selection--;
        break;
      case KEY_DOWN :
        selection == numOptions - 1 ? selection = 0 : selection++;
        break;
      case 'q' :  /* Quit key */
        exitUos();
        exit( EXIT_SUCCESS );
      default:
        break;
    }
  }
}

void exitUos() {
  free(hacks);
  free(messages);

  endwin();  /* End ncurses mode */
}

void lockTerminal() {
  /* TODO: Find a way to move the entire screen up, line by line until no
           lines are visible. */

  curs_set(0);  /* Hide the cursor */

  erase();

  int centerX = cols/2;
  int centerY = rows/2;

  int len = (int) strlen("TERMINAL LOCKED");
  mvprintw(centerY-5, centerX-(len/2), "TERMINAL LOCKED");
  len = (int) strlen("PLEASE CONTACT AN ADMINISTRATOR");
  mvprintw(centerY-3, centerX-(len/2), "PLEASE CONTACT AN ADMINISTRATOR");

  getch();
  exitUos();
}

void manualInputMode() {
  int currentLine = 5;

  mvprintw(currentLine, 1, ">");

  char str[80];  /* Nobody should write any command over 80 chars long! */

  boolean inStartup = TRUE;

  while (inStartup) {

    getstr( str );

    if ( strcmp( str, "SET TERMINAL/INQUIRE" ) == 0 ) {
      currentLine += 2;
      mvprintw(currentLine, 1, "RIT-V300" );
      currentLine += 2;
    } else if ( strcmp( str, "SET FILE/PROTECTION=OWNER:RWED ACCOUNTS.F" ) == 0) {
      currentLine++;
    } else if ( strcmp( str, "SET HALT RESTART/MAINT" ) == 0 ) {
      currentLine += 2;
      mvtermprint( currentLine++, 1, "Initializing Robco Industries(TM) MF Boot Agent v2.3.0", PRINT_SPEED);
      mvtermprint( currentLine++, 1, "RETROS BIOS", PRINT_SPEED);
      mvtermprint( currentLine++, 1, "RBIOS-4.02.08.00 52EE5.E7.E8", PRINT_SPEED);
      mvtermprint( currentLine++, 1, "Copyright 2201-2203 Robco Ind.", PRINT_SPEED);
      mvtermprint( currentLine++, 1, "Uppermem: 64 KB", PRINT_SPEED);
      mvtermprint( currentLine++, 1, "Root (5A8)", PRINT_SPEED);
      mvtermprint(currentLine, 1, "Maintenance Mode", PRINT_SPEED);
      currentLine += 2;
    } else if ( strcmp( str, "RUN DEBUG/ACCOUNTS.F" ) == 0 ) {
      inStartup = FALSE;
    } else if ( strcmp( str, "EXIT" ) == 0 ) {
      currentLine += 2;
      mvtermprint(currentLine, 1, "EXITING...", PRINT_SPEED );
      refresh();
      usleep(1000000);
      exitUos();
    } else {
      currentLine += 2;

      char builder[80];
      builder[0] = 'U';
      builder[1] = 'O';
      builder[2] = 'S';
      builder[3] = ':';
      builder[4] = ' ';

      int count = 5;

      while ( (builder[count] = str[count-5]) ) {
        count++;
      }
      builder[count++] = ':';
      builder[count++] = ' ';
      builder[count++] = 'c';
      builder[count++] = 'o';
      builder[count++] = 'm';
      builder[count++] = 'm';
      builder[count++] = 'a';
      builder[count++] = 'n';
      builder[count++] = 'd';
      builder[count++] = ' ';
      builder[count++] = 'n';
      builder[count++] = 'o';
      builder[count++] = 't';
      builder[count++] = ' ';
      builder[count++] = 'f';
      builder[count++] = 'o';
      builder[count++] = 'u';
      builder[count++] = 'n';
      builder[count++] = 'd';
      builder[count] = '\0';

      mvprintw(currentLine, 1, "%s", builder );
      currentLine += 2;
    }

    mvprintw(currentLine, 1, ">");
  }
}

void autoInputMode() {
  mvtermprint( 5, 1, "SET TERMINAL/INQUIRE", TYPE_SPEED);

  mvtermprint( 7, 1, "RIT-V300", PRINT_SPEED);

  mvtermprint( 9, 1, "SET FILE/PROTECTION=OWNER:RWED ACCOUNTS.F", TYPE_SPEED);

  mvtermprint( 10, 1, "SET HALT RESTART/MAINT", TYPE_SPEED);

  mvtermprint( 12, 1, "Initializing Robco Industries(TM) MF Boot Agent v2.3.0", PRINT_SPEED);
  mvtermprint( 13, 1, "RETROS BIOS", PRINT_SPEED);
  mvtermprint( 14, 1, "RBIOS-4.02.08.00 52EE5.E7.E8", PRINT_SPEED);
  mvtermprint( 15, 1, "Copyright 2201-2203 Robco Ind.", PRINT_SPEED);
  mvtermprint( 16, 1, "Uppermem: 64 KB", PRINT_SPEED);
  mvtermprint( 17, 1, "Root (5A8)", PRINT_SPEED);
  mvtermprint( 18, 1, "Maintenance Mode", PRINT_SPEED);

  mvtermprint( 20, 1, "RUN DEBUG/ACCOUNTS.F", TYPE_SPEED);
}

int calculateNextCurX(int keyDirection) {
  int startOfWord = insideWord();
  int wordStartLine = startOfWord /12;
  int currentArrayPosition = arrayFromYX(cur.y, cur.x);
  int returnVal;

  /* TODO: Refactor the code below. There must be a better way! */
  if ( keyDirection == KEY_LEFT ) {                      /* Left key was pressed. */
    if ( currentArrayPosition <= 203 ) {                  /*   Cursor is on the left side of the board. */
      if ( currentArrayPosition /12 == wordStartLine) {   /*     Cursor is on same line as start of word. */
        if ( startOfWord - 1 >= wordStartLine * 12)
          returnVal = arrayToPoint(startOfWord).x - 1;
        else
          returnVal = cur.x;
      } else {                                            /*     Cursor is not on same line as start of word. */
        returnVal = cur.x;
      }
    } else {                                              /*    Cursor is on the right side of the board. */
      if ( currentArrayPosition /12 == wordStartLine) {   /*      Cursor is on same line as start of word. */
        if (arrayToPoint(startOfWord).x == 28)
          returnVal = 19;
        else
          returnVal = arrayToPoint(startOfWord).x - 1;
      } else {                                            /*      Cursor is not on same line as start of word. */
        returnVal = 19;
      }
    }
  } else {                                                              /* Right key was pressed */
    if ( currentArrayPosition <= 203 ) {                                /*   Cursor is on the left side of the board. */
      if ( currentArrayPosition /12 == wordStartLine) {                 /*     Cursor is on same line as start of word. */
        if ( wordStartLine != (startOfWord + passwordLength)/12)
          returnVal = 28;
        else
          returnVal = arrayToPoint(insideWord()).x + passwordLength;
      } else {                                                          /* Cursor is not on same line as start of word. */
        returnVal = START_X + passwordLength - (12 - ( startOfWord - ( wordStartLine * 12 ) ) );
      }
    } else {                                                            /* Cursor is on the right side of the board. */
      if ( currentArrayPosition /12 == wordStartLine) {                 /* Cursor is on same line as start of word. */
        if ( wordStartLine == (startOfWord + passwordLength)/12)
          returnVal = arrayToPoint(insideWord()).x + passwordLength;
        else
          returnVal = cur.x;
      } else {                                                          /* Cursor is not on same line as start of word. */
        returnVal = START_X + 20 + passwordLength - (12 - ( startOfWord - ( wordStartLine * 12 ) ) );
      }
    }
  }

  return returnVal;
}

void refreshPasswords() {
  /* At each passLocation copy a password. */
  int currLocation = 0;
  int i;
  for ( i = 0; i < NUM_PASSWORDS; i++ ) {
    currLocation = passwords[i]->position;

    char *s = passwords[i]->content;
    char c;
    int j = 0;
    while ( (c = s[j]) ) {
      board[currLocation] = c;

      j++;
      currLocation++;
    }
  }
}

char *uniqueRandomWord() {
  /* TODO: Make each work unique. */

  /* Pick a word from 0 - 39 */
  return wordList[ rand() % 40 ];
}