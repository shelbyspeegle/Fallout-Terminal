/*
 * hackterm.c
 *
 *  Created on: May 3, 2014
 *      Author: shelbyspeegle
 */

#define _BSD_SOURCE                            /* My Linux machine needs this */

#include <ncurses.h>  /* ncurses.h includes stdio.h */
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
char **wordlist;
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
int passwordsleftonboard = NUM_PASSWORDS;
FILE *fr;

void setup();
void printinputarea();
void refreshboard();
void pushmessage( const char *newMessage );
boolean trypassword();
void genpasswords();
void genhacks();
int insideword();
int insidehack();
void highlight();
char * stringatcursor();
void mvtermprint( int y, int x, char *string, int speed );
int numberofcorrectchars( const char *checkword );
void accesssystem();
void exituos();
void lockterminal();
void manualinputmode();
void autoinputmode();
int calculatenextcurx( int key_direction );
void refreshpasswords();
char * uniquerandomword();

int main( int argc, char **argv ) {

  debug = (argc == 2 && argv[1][0] == 'd');

  srand( time(0) );  /* Seed rand with this so it is more random. */

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

  wordlist = malloc( sizeof(char *) * 40 );

  /* Word lengths between 4-15 */

  char *filename = (char *) malloc( sizeof(char) * 8 );

  sprintf( filename, "%i.txt", passwordLength);

  fr = fopen(filename, "r");

  char *line = NULL;
  size_t linecapp = 100;

  int m = 0;

  for ( m=0; m<40; m++ ) {
    getline( &line, &linecapp, fr );
    wordlist[m] = malloc( sizeof(char) * passwordLength );
    strncpy( wordlist[m], line, passwordLength );
  }

  free(line);
  fclose(fr);

  /* Cut to the chase if debug mode is on. */
  if ( debug ) TYPE_SPEED = PRINT_SPEED = 0;

  /* Start of display. */
  mvtermprint( 1, 1, "SECURITY RESET...", PRINT_SPEED);
  clear();
  mvtermprint( 3, 1, "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK", PRINT_SPEED);

  hardmode ? manualinputmode() : autoinputmode();

  setup();

  boolean loggedin = FALSE;

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
        lockterminal();
    }

    mvprintw(4, 1, "%i", trysLeft);

    refreshboard();

    move(cur.y, cur.x);

    highlight();
    refresh();

    printinputarea();

    if (loggedin) {
      usleep(1000000*3);
      accesssystem();
    }

    int uInput = getch();  /* Get user input from keyboard (pause) */
    usleep(10);  /* Reduces cursor jump if arrows are held down */

    switch (uInput) {
      case '\n' :  /* TODO: test Linux */
        loggedin = trypassword();
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
        if (insideword() > -1 )                /* If the cursor is inside a word-                            */
          cur.x = calculatenextcurx( uInput );  /*   figure out the next cur.x location.                      */
        else if (cur.x == 28)                   /* Else if the cursor is at the left bound of the right side- */
          cur.x = 19;                           /*   move the cursor to the right bound of the left side.     */
        else if (cur.x != 8)                    /* As long as the cursor is not at the far left of the board- */
          cur.x--;                              /*   move the cursor left.                                    */
        break;
      case KEY_RIGHT :
        if (insideword() > -1 )                /* If the cursor is inside a word-                             */
          cur.x = calculatenextcurx( uInput );  /*   figure out the next cur.x location.                       */
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
          pushmessage("Allowance");
          pushmessage("replenished.");
        }
        break;
      case 'a' :
        loggedin = TRUE;
        break;
      case 'q' :  /* Quit key */
        exituos();
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

  genpasswords();
  refreshpasswords();
  genhacks();

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
      usleep(PRINT_SPEED/6);
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
      usleep(PRINT_SPEED/6);
      refresh();
    }

    printX = 21;
    printY++;
  }
}

void printinputarea() {
  int i;
  for (i=0; i < MAX_MESSAGES; i++) {
    if (messages[i])
      mvprintw(20-i, 41, "%s", messages[i]);  /* Print messages that exist */
  }

  mvprintw( 22, 42, "              ");  /* Clear input line */

  mvtermprint( 22, 42, stringatcursor(), PRINT_SPEED );
}

void refreshboard() {
  int i;
  for (i = 0; i < 408; i++) {  /* iterate through each char in array */
    mvprintw(arraytopoint(i).y, arraytopoint(i).x, "%c", board[i]);
  }
}

void pushmessage( const char *newMessage ) {
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

boolean trypassword() {
  if (insideword() >= 0) {  /* Test if the cursor on a Password. */
    int k = insideword();  /* Set k to the start of the word the cursor is in. */
    char *check = malloc(sizeof(char) * passwordLength);
    int j;
    for (j=0; j<passwordLength; j++) {
      check[j] = board[k++];
    };

    pushmessage( stringatcursor() );

    int i = numberofcorrectchars(check);
    if (i == passwordLength) {
      pushmessage( "Exact match!" );
      pushmessage( "Please wait" );
      pushmessage( "while system" );
      pushmessage( "is accessed." );

      return TRUE;
    } else {
      pushmessage( "Entry denied" );
      char *stringbuild = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
      sprintf(stringbuild, "%i/%i correct.", i, passwordLength);
      pushmessage( stringbuild );
      trysLeft--;
    }
  } else if (insidehack() >= 0) {  /* Test if the cursor on a Hack. */

    int a = insidehack();

    if (passwordsleftonboard > 1) {  /* More than one Password on the board. */

      int i;
      for (i = 0; i < NUM_HACKS; i++) {  /* Iterate through all words. */
        if (a == hacks[i]->position ) {
          pushmessage( hacks[i]->content );
          hacks[i]->position = -1;  /* Make this hack unusable. */
        }
      }

      pushmessage("Dud removed.");

      boolean flag = FALSE;

      /* TODO maybe try a do-while here. */
      while ( !flag && passwordsleftonboard > 1 ) {  /* Search for incorrect password to remove as a dud. */
        int min = 0;
        int max = NUM_PASSWORDS-1;

        int i = ( rand() % (max+1-min) ) + min;  /* Pick a random incorrect Password. */

        if (!passwords[i]->correct && !passwords[i]->removed) {

          /* Copy over contents of incorrect password with dots. */
          char *pointer = passwords[i]->content;
          passwords[i]->removed = TRUE;
          passwordsleftonboard--;

          int j = 0;
          while (pointer[j] != '\0') {
            pointer[j] = '.';
            j++;
          }

          flag = TRUE;
        }
      }

      refreshpasswords();

      /* TODO provide logic for allowance replenishing */

    } else {  /* Only one password is left on the board. */

      /* TODO refactor this. It is a serious waste. */

      int i;
      for (i = 0; i < NUM_HACKS; i++) { /* iterate through all words */
        if (a == hacks[i]->position ) {
          pushmessage( hacks[i]->content );
        }
      }

      pushmessage("Entry denied");
    }
  } else { /* The cursor is on Trash. */
    pushmessage( stringatcursor() );
    pushmessage( "Entry denied" );

    char *stringbuild = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
    sprintf(stringbuild, "0/%i correct.", passwordLength);
    pushmessage( stringbuild );
    trysLeft--;
  }

  return FALSE;
}

void genhacks() {
  /* TODO: Add this logic... If hack is placed inside another, set the contents of the hack we are inside to the hack that is inside. */

  int newhackpositions[NUM_HACKS];
  int hackrow, hackrowposition, hackstart, hackend, passwordstart, passwordend;

  int i;
  for ( i=0; i<NUM_HACKS; i++ ) {

    hacks[i] = createHack();

    boolean validarrayposition = FALSE;

    while ( !validarrayposition ) {

      validarrayposition = TRUE; /* TODO: fix... TOO HACKY! */

      /* Pick a row from 0 - 33 */
      hackrow = rand() % 34;

      /* Pick a spot in the row so that the hack remains on a single line. */
      hackrowposition = rand() % ( 12  - hacks[i]->size ) ;

      hackstart = ( hackrow*12 ) + hackrowposition;
      hackend = hackstart + hacks[i]->size;

      int j;
      /* Make sure the hack does not collide with any passwords. */
      for ( j=0; j<NUM_PASSWORDS; j++ ) {
        passwordstart = passwords[j]->position;
        passwordend = passwordstart + passwords[j]->size;

        validarrayposition = validarrayposition && ( hackend < passwordstart || hackstart > passwordend );

        if ( !validarrayposition ) {
          break; /* We do not need to look any further. */
        }
      }

      /* If hack collides with other hack, make sure that it is inside. */
      for ( j=0; j<i; j++ ) {
        /* Position is not valid if our hack starts at the same spot as another. */
        if ( hackstart == hacks[j]->position ) {
          validarrayposition = FALSE;
          break;
        }

        /* Hacks cannot overlap. */
        if ( hackstart < hacks[j]->position && hackend < hacks[j]->position + hacks[j]->size && hackend > hacks[j]->position ) {
          validarrayposition = FALSE;
          break;
        }

        /* Hack can be placed inside another hack. */
        if ( hackstart > hacks[j]->position && hackstart < hacks[j]->position + hacks[j]->size ) {
          boolean useSameBracket = sameBracketType( hacks[i], hacks[j] );
          boolean completelyInside = hackend < hacks[j]->position + hacks[j]->size;
          boolean shareClosingBracket = hackend == hacks[j]->position + hacks[j]->size;

          /* As long as it is nested completely inside and the hacks don't use the same brackets. */
          /* Or if hacks use the same closing bracket they may share that. */
          if ( (completelyInside && !useSameBracket) || (shareClosingBracket && useSameBracket) ) {
            validarrayposition = validarrayposition && TRUE;
          } else {
            validarrayposition = FALSE;
            break;
          }
        }
      }
    }

    /* Safe spot for hack successfully found! */
    newhackpositions[i] = hackstart;
  }

  /* Set the locations of the newly created hacks. */
  for ( i=0; i<NUM_HACKS; i++ ) {
    setHackPosition( hacks[i], newhackpositions[i] );
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

void genpasswords() {  /* Fill the passwords array with Passwords */

  int max = 407-1-passwordLength;
  int min = 0;
  int newpasswordpositions[NUM_PASSWORDS];

  int i;
  for ( i=0; i<NUM_PASSWORDS; i++ ) {
    boolean notavalidposition = TRUE;
    int positionfornewpassword;
    while ( notavalidposition ) {
      positionfornewpassword = ( rand() % (max+1-min) ) + min;  /* Pick a spot on the board */

      /* TODO: This is hacky. Setting flag to FALSE until something below trips it to TRUE */
      notavalidposition = FALSE;

      int j;
      for ( j=0; j<i; j++ ) {  /* Make sure current position is clear of every word generated */
        if ( positionfornewpassword < newpasswordpositions[j] + passwordLength + 1
            && positionfornewpassword+passwordLength > newpasswordpositions[j]) {
          notavalidposition = TRUE;
        }
      }
    }

    /* Place password at positionfornewpassword */
    newpasswordpositions[i] = positionfornewpassword;
  }

  int j;
  for ( j=0; j<NUM_PASSWORDS; j++ ) {
    passwords[j] = createPassword( uniquerandomword(), newpasswordpositions[j] );
  }

  /* Pick one of the passwords on the board to be correct. */
  max = NUM_PASSWORDS-1;
  min = 0;
  correct = ( rand() % (max+1-min) ) + min;  /* 0, NUM_PASSWORDS-1 */
  passwords[correct]->correct = TRUE;
}

int insideword() {  /* if inside word, return array start position, else -1 */
  int a = yxtoarray(cur.y, cur.x);

  int i;
  for (i = 0; i < NUM_PASSWORDS; i++) {  /* iterate through all words */
    if (a >= passwords[i]->position && a < passwords[i]->position + passwordLength ) {
      return passwords[i]->position;
    }
  }

  return -1;
}

int insidehack() {  /* if inside word, return array start position, else -1 */
  int a = yxtoarray(cur.y, cur.x);

  int i;
  for (i = 0; i < NUM_HACKS; i++) {  /* iterate through all words */
    if (a == hacks[i]->position )
      return hacks[i]->position;
  }

  return -1;
}

void highlight() {
  attron(A_STANDOUT);

  int a = yxtoarray(cur.y, cur.x);

  if (insideword() >= 0) {

    /* TODO Do removed passwords highlight? */

    a = insideword();
    int i;
    for (i = 0; i < NUM_PASSWORDS; i++) {  /* iterate through all words */
      if (a == passwords[i]->position ) {
        for ( i=0; i<passwords[i]->size; i++) {
          mvprintw( arraytopoint(a).y, arraytopoint(a).x, "%c", board[a]);
          a++;
        }
        break;
      }
    }

    attroff(A_STANDOUT);
    return;
  } else if (insidehack() >= 0) {
    a = insidehack();

    int i;
    for (i = 0; i < NUM_HACKS; i++) {  /* iterate through all words */
      if (a == hacks[i]->position ) {
        int j;
        for ( j=0; j<hacks[i]->size; j++) {
          mvprintw( arraytopoint(a).y, arraytopoint(a).x, "%c", board[a]);
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

char * stringatcursor() {  /* TODO: Try refactoring this. */
  if (insideword() >= 0) {  /* Cursor is in word. */
    char *check = malloc(sizeof(char) * passwordLength);

    int k = insideword();

    int i;
    for ( i=0; i<passwordLength; i++ ) {
      check[i] = board[k++];
    };

    return check;
  } else if (insidehack() >= 0) {

    /* Find the hack that we are inside. */
    int positionofhack = insidehack();

    int i;
    for ( i=0; i< NUM_HACKS; i++) {
      if (hacks[i]->position == positionofhack) {
        return hacks[i]->content;
      }
    }
  }

  /* Cursor is on a single char. */
  char *returner = malloc(sizeof(char) * 2);
  returner[0] = board[yxtoarray(cur.y, cur.x)];
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
  int len = strlen(string);

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
      usleep(speed);
    }
  }

  if (speed == TYPE_SPEED && speed > 0) {
    usleep(1000000);
  }
}

int numberofcorrectchars( const char *checkword ) {
  if (strlen(checkword) != passwordLength) {
    /* TODO: properly free memory and end the program */
    endwin();
    fprintf(stderr, "ERROR: numberofcorrectchars called on string of length %i when passwords are of length %i.\n",
        (int) strlen(checkword), passwordLength);
    exit(1);
  }

  int count = 0;

  int i;
  for (i=0; i < passwordLength; i++) {
    if (passwords[correct]->content[i] == checkword[i]) {
      count++;
    }
  }

  return count;
}

void accesssystem() {
  int print_speed_fast = PRINT_SPEED/3;

  clear();

  mvtermprint( 1, 1, "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK", print_speed_fast );
  mvtermprint( 3, 1, " LOGIN ADMIN", print_speed_fast );
  mvtermprint( 5, 1, "ENTER PASSWORD NOW", print_speed_fast );

  mvprintw( 7, 1, ">");
  refresh();
  usleep(1000000);  /* TODO: get exact time */
  int i;
  for (i=0; i < passwordLength; i++) {
    mvprintw(7, 3+i, "*");
    usleep(TYPE_SPEED*3);
    refresh();
  }
  usleep(1000000);  /* TODO: get exact time */

  clear();

  mvtermprint( 1, 7, "ROBCO INDUSTRIES UNIFIED OPERATING SYSTEM", print_speed_fast );
  mvtermprint( 2, 9, "COPYRIGHT 2075-2077 ROBCO INDUSTRIES", print_speed_fast );
  mvtermprint( 3, 22, "-Server 1-", print_speed_fast );

  /* If Lock ///////////////////////////////////////////////////////////////*/
  mvtermprint( 5, 9, "SoftLock Solutions, Inc", print_speed_fast );
  mvtermprint( 6, 5, "\"Your Security is Our Security\"", print_speed_fast );
  mvtermprint( 7, 5, ">\\ Welcome, USER", print_speed_fast );
  mvtermprint( 8, 5, "_______________________________", print_speed_fast );

  int selection = 0;

  int num_options = 2;
  char *menu[] = {
      "Disengage Lock",
      "Exit"
  };

  /* Menu */
  while (1) {
    /* Print Menu */
    int i;
    for (i=0; i < num_options; i++) {
      if (selection == i) {
        attron(A_STANDOUT);
        mvprintw( 9+i, 5, "                                            ");
        mvprintw( 9+i, 5, "> %s", menu[i]);
        attroff(A_STANDOUT);
      } else {
        mvprintw( 9+i, 5, "                                            ");
        mvprintw(9+i, 5, "> %s", menu[i]);
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
          exituos();
        }
        break;
      case KEY_UP :
        selection == 0 ? selection = 1 : selection--;
        break;
      case KEY_DOWN :
        selection == num_options - 1 ? selection = 0 : selection++;
        break;
      case 'q' :  /* Quit key */
        exituos();
      default:
        break;
    }
  }
}

void exituos() {
  free(hacks);
  free(messages);

  endwin();  /* End ncurses mode */
  exit( EXIT_SUCCESS );
}

void lockterminal() {
  /* TODO: Find a way to move the entire screen up, line by line until no
           lines are visible. */

  curs_set(0);  /* Hide the cursor */

  erase();

  int centerX = cols/2;
  int centerY = rows/2;

  int len = strlen("TERMINAL LOCKED");
  mvprintw(centerY-5, centerX-(len/2), "TERMINAL LOCKED");
  len = strlen("PLEASE CONTACT AN ADMINISTRATOR");
  mvprintw(centerY-3, centerX-(len/2), "PLEASE CONTACT AN ADMINISTRATOR");

  getch();
  exituos();
}

void manualinputmode() {
  int curLine = 5;

  mvprintw( curLine, 1, ">");

  char str[80];  /* Nobody should write any command over 80 chars long! */

  boolean instartup = TRUE;

  while ( instartup ) {

    getstr( str );

    if ( strcmp( str, "SET TERMINAL/INQUIRE" ) == 0 ) {
      curLine += 2;
      mvprintw( curLine, 1, "RIT-V300" );
      curLine += 2;
    } else if ( strcmp( str, "SET FILE/PROTECTION=OWNER:RWED ACCOUNTS.F" ) == 0) {
      curLine++;
    } else if ( strcmp( str, "SET HALT RESTART/MAINT" ) == 0 ) {
      curLine += 2;
      mvtermprint( curLine++, 1, "Initializing Robco Industries(TM) MF Boot Agent v2.3.0", PRINT_SPEED);
      mvtermprint( curLine++, 1, "RETROS BIOS", PRINT_SPEED);
      mvtermprint( curLine++, 1, "RBIOS-4.02.08.00 52EE5.E7.E8", PRINT_SPEED);
      mvtermprint( curLine++, 1, "Copyright 2201-2203 Robco Ind.", PRINT_SPEED);
      mvtermprint( curLine++, 1, "Uppermem: 64 KB", PRINT_SPEED);
      mvtermprint( curLine++, 1, "Root (5A8)", PRINT_SPEED);
      mvtermprint( curLine, 1, "Maintenance Mode", PRINT_SPEED);
      curLine += 2;
    } else if ( strcmp( str, "RUN DEBUG/ACCOUNTS.F" ) == 0 ) {
      instartup = FALSE;
    } else if ( strcmp( str, "EXIT" ) == 0 ) {
      curLine += 2;
      mvtermprint( curLine, 1, "EXITING...", PRINT_SPEED );
      refresh();
      usleep(1000000);
      exituos();
    } else {
      curLine += 2;

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

      mvprintw( curLine, 1, "%s", builder );
      curLine += 2;
    }

    mvprintw(curLine, 1, ">");
  }
}

void autoinputmode() {
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

int calculatenextcurx( int key_direction ) {
  int startOfWord = insideword();
  int wordStartLine = startOfWord /12;
  int currentArrayPosition = yxtoarray( cur.y, cur.x );
  int returnVal;

  /* TODO: Refactor the code below. There must be a better way! */
  if ( key_direction == KEY_LEFT ) {                      /* Left key was pressed. */
    if ( currentArrayPosition <= 203 ) {                  /*   Cursor is on the left side of the board. */
      if ( currentArrayPosition /12 == wordStartLine) {   /*     Cursor is on same line as start of word. */
        if ( startOfWord - 1 >= wordStartLine * 12)
          returnVal = arraytopoint(startOfWord).x - 1;
        else
          returnVal = cur.x;
      } else {                                            /*     Cursor is not on same line as start of word. */
        returnVal = cur.x;
      }
    } else {                                              /*    Cursor is on the right side of the board. */
      if ( currentArrayPosition /12 == wordStartLine) {   /*      Cursor is on same line as start of word. */
        if ( arraytopoint(startOfWord).x == 28)
          returnVal = 19;
        else
          returnVal = arraytopoint(startOfWord).x - 1;
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
          returnVal = arraytopoint(insideword() ).x + passwordLength;
      } else {                                                          /* Cursor is not on same line as start of word. */
        returnVal = START_X + passwordLength - (12 - ( startOfWord - ( wordStartLine * 12 ) ) );
      }
    } else {                                                            /* Cursor is on the right side of the board. */
      if ( currentArrayPosition /12 == wordStartLine) {                 /* Cursor is on same line as start of word. */
        if ( wordStartLine == (startOfWord + passwordLength)/12)
          returnVal = arraytopoint(insideword() ).x + passwordLength;
        else
          returnVal = cur.x;
      } else {                                                          /* Cursor is not on same line as start of word. */
        returnVal = START_X + 20 + passwordLength - (12 - ( startOfWord - ( wordStartLine * 12 ) ) );
      }
    }
  }

  return returnVal;
}

void refreshpasswords() {
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

char * uniquerandomword() {
  /* TODO: Make each work unique. */

  /* Pick a word from 0 - 39 */
  return wordlist[ rand() % 40 ];
}