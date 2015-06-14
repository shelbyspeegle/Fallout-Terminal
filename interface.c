#include <string.h>
#include "interface.h"

#include "game_controller.h"
#include "utilities.h"

#define BOARD_WIDTH 55
#define BOARD_HEIGHT 24

useconds_t TYPE_SPEED = 24000;
useconds_t PRINT_SPEED = 18000;
useconds_t TYPE_DELAY = 1000000;

char *registers[] = {
    "0xF964", "0xF970", "0xF97C", "0xF988", "0xF994", "0xF9A0", "0xF9AC",
    "0xF9B8", "0xF9C4", "0xF9D0", "0xF9DC", "0xF9E8", "0xF9F4", "0xFA00",
    "0xFA0C", "0xFA18", "0xFA24", "0xFA30", "0xFA3C", "0xFA48", "0xFA54",
    "0xFA60", "0xFA6C", "0xFA78", "0xFA84", "0xFA90", "0xFA9C", "0xFAA8",
    "0xFA84", "0xFAC0", "0xFACC", "0xFAD8", "0xFAE4", "0xFAF0"
};

void initializeInterface() {
  initscr();  /* Start ncurses mode. */
  getmaxyx( stdscr, rows, cols );  /* Capture the terminal window size. */
  keypad( stdscr, TRUE );  /* Converts arrow key input to usable chars. */

  terminalStartX = cols/2 - BOARD_WIDTH/2;
  terminalStartY = rows/2 - BOARD_HEIGHT/2;

  /* Cut to the chase if debug mode is on. */
  if ( getDebugStatus() ) {
    TYPE_SPEED = TYPE_SPEED/20;
    PRINT_SPEED = PRINT_SPEED/20;
    TYPE_DELAY = TYPE_DELAY/20;
  }
}

void refreshBoard() {
  char * boardPtr = getBoard();

  if ( getDebugStatus() ) {
    mvaddch( terminalStartY - 1, terminalStartX - 1, ACS_ULCORNER );
    mvaddch( terminalStartY - 1, terminalStartX + BOARD_WIDTH, ACS_URCORNER );
    mvaddch( terminalStartY + BOARD_HEIGHT, terminalStartX - 1, ACS_LLCORNER );
    mvaddch( terminalStartY + BOARD_HEIGHT, terminalStartX + BOARD_WIDTH, ACS_LRCORNER );
  }
  int i;
  for (i = 0; i < 408; i++) {  /* iterate through each char in array */
    mvprintw(terminalStartY + arrayToPoint(i).y, terminalStartX + arrayToPoint(i).x, "%c", *(boardPtr + i) );
  }
}

int terminalTooSmall() {
  return (rows < BOARD_HEIGHT || cols < BOARD_WIDTH);
}

void error_terminalTooSmall() {
  endwin();
  printf("ERROR: Terminal window is too small,\n");
  printf("       minimum size of %ix%i to run.\n", BOARD_WIDTH , BOARD_HEIGHT);
}

void mvtermprint( int y, int x, char *string, useconds_t speed ) {
  int i;
  int len = (int) strlen(string);

  if (speed == TYPE_SPEED && speed > 0) {
    mvprintw( y, x++, ">" );
    refresh();

    usleep(TYPE_DELAY);
  }

  if (len == 1) {
    mvprintw( y, x++, "%c", string[0] );
    refresh();
  } else {
    for (i=0; i < len; i++) {
      mvprintw( y, x++, "%c", string[i] );
      refresh();
      usleep( speed );
    }
  }

  if (speed == TYPE_SPEED && speed > 0) {
    usleep(TYPE_DELAY);
  }
}

void printScreen1() {
  /* Start of display. */
  mvtermprint( terminalStartY + 1, terminalStartX + 1, "SECURITY RESET...", PRINT_SPEED );
  clear();
  mvtermprint( terminalStartY + 3, terminalStartX + 1, "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK", PRINT_SPEED );
}

void printScreen2() {
  if ( getHardmodeStatus() ) {
    int currentLine = 5;

    curs_set(1);

    mvprintw( terminalStartY + currentLine, terminalStartX + 1, ">" );

    char str[80];  /* Nobody should write any command over 80 chars long! */

    boolean inStartup = TRUE;

    /* TODO: Include buffer containing all lines entered, so that we can move the old messages up when the screen becomes full. */

    while (inStartup) {

      getstr( str );

      if ( strcmp( str, "HELP" ) == 0 ) {
        currentLine += 2;
        mvtermprint( terminalStartY + currentLine++, terminalStartX + 1,  "ROBCO INDUSTRIES (TM) TERMLINK", PRINT_SPEED );
        mvtermprint( terminalStartY + currentLine, terminalStartX + 1, "RBIOS-4.02.08.00 52EE5.E7.E8", PRINT_SPEED );
        currentLine += 2;
        mvtermprint( terminalStartY + currentLine++, terminalStartX + 1 + 3, "SET", PRINT_SPEED );
        mvtermprint( terminalStartY + currentLine++, terminalStartX + 1 + 3, "RUN", PRINT_SPEED );
        mvtermprint( terminalStartY + currentLine, terminalStartX + 1 + 3, "EXIT", PRINT_SPEED );
        currentLine += 2;
      } else if ( strstr(str, "SET") != NULL ) {
        if ( strcmp( str, "SET TERMINAL/INQUIRE" ) == 0 ) {
          currentLine += 2;
          mvprintw(terminalStartY + currentLine, terminalStartX + 1, "RIT-V300");
          currentLine += 2;
        } else if ( strcmp( str, "SET FILE/PROTECTION=OWNER:RWED ACCOUNTS.F" ) == 0) {
          currentLine++;
        } else if ( strcmp( str, "SET HALT RESTART/MAINT" ) == 0 ) {
          currentLine += 2;
          mvtermprint( terminalStartY + currentLine++, terminalStartX + 1, "Initializing Robco Industries(TM) MF Boot Agent v2.3.0", PRINT_SPEED );
          mvtermprint( terminalStartY + currentLine++, terminalStartX + 1, "RETROS BIOS", PRINT_SPEED );
          mvtermprint( terminalStartY + currentLine++, terminalStartX + 1, "RBIOS-4.02.08.00 52EE5.E7.E8", PRINT_SPEED );
          mvtermprint( terminalStartY + currentLine++, terminalStartX + 1, "Copyright 2201-2203 Robco Ind.", PRINT_SPEED );
          mvtermprint( terminalStartY + currentLine++, terminalStartX + 1, "Uppermem: 64 KB", PRINT_SPEED );
          mvtermprint( terminalStartY + currentLine++, terminalStartX + 1, "Root (5A8)", PRINT_SPEED );
          mvtermprint( terminalStartY + currentLine, terminalStartX + 1, "Maintenance Mode", PRINT_SPEED );
          currentLine += 2;
        } else if ( strcmp( str, "SET --HELP" ) == 0 ) {
          currentLine += 2;
          mvtermprint(terminalStartY + currentLine++, terminalStartX + 1 + 3, "TERMINAL/INQUIRE", PRINT_SPEED );
          mvtermprint(terminalStartY + currentLine++, terminalStartX + 1 + 3, "FILE/PROTECTION=OWNER:RWED ACCOUNTS.F", PRINT_SPEED );
          mvtermprint(terminalStartY + currentLine, terminalStartX + 1 + 3, "HALT RESTART/MAINT", PRINT_SPEED );
          currentLine += 2;
        } else {
          currentLine++;
          mvprintw(terminalStartY + currentLine, terminalStartX + 1, "USAGE: SET <COMMAND> [--HELP]");
          currentLine += 2;
        }
      } else if ( strcmp( str, "RUN DEBUG/ACCOUNTS.F" ) == 0 ) {
        inStartup = FALSE;
      } else if ( strcmp( str, "EXIT" ) == 0 ) {
        currentLine += 2;
        mvtermprint( terminalStartY + currentLine, terminalStartX + 1, "EXITING...", PRINT_SPEED );
        refresh();
        usleep(TYPE_DELAY);
        exitUos( EXIT_SUCCESS );
      } else {
        currentLine += 2;

        char builder[80];
        int count = 0;

        while ( (builder[count] = str[count]) ) {
          count++;
        }
        builder[count++] = ':';
        builder[count++] = ' ';
        builder[count++] = 'C';
        builder[count++] = 'O';
        builder[count++] = 'M';
        builder[count++] = 'M';
        builder[count++] = 'A';
        builder[count++] = 'N';
        builder[count++] = 'D';
        builder[count++] = ' ';
        builder[count++] = 'N';
        builder[count++] = 'O';
        builder[count++] = 'T';
        builder[count++] = ' ';
        builder[count++] = 'F';
        builder[count++] = 'O';
        builder[count++] = 'U';
        builder[count++] = 'N';
        builder[count++] = 'D';
        builder[count] = '\0';

        mvprintw( terminalStartY + currentLine, terminalStartX + 1, "%s", builder );
        currentLine += 2;
        mvprintw( terminalStartY + currentLine, terminalStartX + 1, "TYPE 'HELP' FOR LISTING OF AVAILABLE OPTIONS." );
        currentLine += 2;
      }

      mvprintw( terminalStartY + currentLine, terminalStartX + 1, ">" );
    }

    curs_set(0);
  } else {
    mvtermprint( terminalStartY + 5, terminalStartX + 1, "SET TERMINAL/INQUIRE", TYPE_SPEED );

    mvtermprint( terminalStartY + 7, terminalStartX + 1, "RIT-V300", PRINT_SPEED );

    mvtermprint( terminalStartY + 9, terminalStartX + 1, "SET FILE/PROTECTION=OWNER:RWED ACCOUNTS.F", TYPE_SPEED );

    mvtermprint( terminalStartY + 10, terminalStartX + 1, "SET HALT RESTART/MAINT", TYPE_SPEED );

    mvtermprint( terminalStartY + 12, terminalStartX + 1, "Initializing Robco Industries(TM) MF Boot Agent v2.3.0", PRINT_SPEED );
    mvtermprint( terminalStartY + 13, terminalStartX + 1, "RETROS BIOS", PRINT_SPEED );
    mvtermprint( terminalStartY + 14, terminalStartX + 1, "RBIOS-4.02.08.00 52EE5.E7.E8", PRINT_SPEED );
    mvtermprint( terminalStartY + 15, terminalStartX + 1, "Copyright 2201-2203 Robco Ind.", PRINT_SPEED );
    mvtermprint( terminalStartY + 16, terminalStartX + 1, "Uppermem: 64 KB", PRINT_SPEED );
    mvtermprint( terminalStartY + 17, terminalStartX + 1, "Root (5A8)", PRINT_SPEED );
    mvtermprint( terminalStartY + 18, terminalStartX + 1, "Maintenance Mode", PRINT_SPEED );

    mvtermprint( terminalStartY + 20, terminalStartX + 1, "RUN DEBUG/ACCOUNTS.F", TYPE_SPEED );
  }
}

void printScreen3() {
  clear();
  noecho();  /* Silence user input */

  /* Board Set-up ////////////////////////////////////////////////////////////*/
  mvprintw( terminalStartY + 22, terminalStartX + 41, ">" );
  refresh();

  /* Place cursor at starting position */
  cur.y = START_Y;
  cur.x = START_X;

  mvtermprint( terminalStartY + 1, terminalStartX + 1, "ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL", PRINT_SPEED );
  mvtermprint( terminalStartY + 2, terminalStartX + 1, "ENTER PASSWORD NOW", PRINT_SPEED );
  mvtermprint( terminalStartY + 4, terminalStartX + 1, "4 ATTEMPT(S) LEFT : ", PRINT_SPEED );
  printRegisters();
}

void updateAttemptsLeft() {
  int attemptsLeft = getAttemptsLeft();

  /* Update the attempts left section. */
  mvprintw( terminalStartY + 4, terminalStartX + 21, "        ");  /* Clear the tries. */

  if (attemptsLeft == 1) {
    attron(A_BLINK);  /* eh blinkin'! */
    mvprintw( terminalStartY + 2, terminalStartX + 1, "!!! WARNING: LOCKOUT IMMINENT !!!" );
    attroff(A_BLINK);
  } else {
    mvprintw( terminalStartY + 2, terminalStartX + 1, "ENTER PASSWORD NOW               " );
  }

  /* Place the symbols for number of tries left. */
  attron(A_STANDOUT);
  int i;
  for ( i = 0; i < attemptsLeft; i++ ) {
    mvprintw( terminalStartY + 4, terminalStartX + 21 + i*2, " " );
  }
  attroff(A_STANDOUT);
  if ( attemptsLeft == 0 ) {
    // TODO: Find a way to move the entire screen up, line by line until no lines are visible.

    curs_set(0);  /* Hide the cursor */

    erase();

    int centerX = BOARD_WIDTH/2;
    int centerY = BOARD_HEIGHT/2;

    int len = (int) strlen("TERMINAL LOCKED");
    mvprintw( terminalStartY + centerY-5, terminalStartX + centerX-(len/2), "TERMINAL LOCKED" );
    len = (int) strlen("PLEASE CONTACT AN ADMINISTRATOR" );
    mvprintw( terminalStartY + centerY-3, terminalStartX + centerX-(len/2), "PLEASE CONTACT AN ADMINISTRATOR" );

    getch();
    exitUos( EXIT_SUCCESS );
  }

  mvprintw( terminalStartY + 4, terminalStartX + 1, "%i", attemptsLeft );

  refreshBoard();

  move(terminalStartY + cur.y, terminalStartX + cur.x);

  highlight();
  refresh();
}

void printInputArea() {
  char ** messagesPtr = getMessages();

  int i;
  for ( i = 0; i < getMaxNumMessages(); i++ ) {
    char * message = *(messagesPtr + i);
    if ( message ) {
      mvprintw(terminalStartY + 20 - i, terminalStartX + 41, "%s", message );  /* Print messages that exist */
    }
  }

  mvprintw( terminalStartY + 22, terminalStartX + 42, "              " );  /* Clear input line */

  mvtermprint( terminalStartY + 22, terminalStartX + 42, stringAtCursor(), PRINT_SPEED );
}

void printRegisters() {
  char * boardPtr = getBoard();

  /* Print Registers /////////////////////////////////////////////////////////*/
  int printX = 1;
  int printY = 6;

  /* Print left half of game */
  int i;
  for (i = 0; i < 17; i++) {
    mvtermprint( terminalStartY + printY, terminalStartX + printX, registers[i], PRINT_SPEED/6 );  /* Print registers */
    printX+=7;
    refresh();

    int j;
    for (j = 0; j < 12; j++) {  /* iterate through each char in array */
      mvprintw( terminalStartY + printY, terminalStartX + printX++, "%c", *(boardPtr + j + (12 * i)) );
      usleep( PRINT_SPEED/6 );
      refresh();
    }

    printX = 1;
    printY++;
  }

  printY = 6;
  printX = 21;

  /* Print right half of game */
  for (i = 0; i < 17; i++) {
    mvtermprint( terminalStartY + printY, terminalStartX + printX, registers[i+17], PRINT_SPEED/6 );  /* Print registers */
    printX+=7;
    refresh();

    int j;
    for (j = 0; j < 12; j++) {  /* iterate through each char in array */
      mvprintw( terminalStartY + printY, terminalStartX + printX++, "%c", *(boardPtr + 204 + j + (12 * i)) );
      usleep( PRINT_SPEED/6 );
      refresh();
    }

    printX = 21;
    printY++;
  }
}

void highlight() {
  char * boardPtr = getBoard();
  PasswordPtr * hacksPtr = getHacks();
  PasswordPtr * passwordsPtr = getPasswords();

  attron(A_STANDOUT);

  int a = arrayFromYX(cur.y, cur.x);

  if (insideWord() >= 0) {

    /* TODO Do removed passwords highlight? */

    a = insideWord();
    int i;
    for (i = 0; i < getNumPasswords(); i++) {  /* iterate through all words */
      PasswordPtr password = *(passwordsPtr + i);

      if (a == password->position ) {
        for ( i=0; i<password->size; i++) {
          mvprintw( terminalStartY + arrayToPoint(a).y, terminalStartX + arrayToPoint(a).x, "%c", *(boardPtr + a) );
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
    for (i = 0; i < getNumHacks(); i++) {  /* iterate through all words */
      PasswordPtr hack = *(hacksPtr + i);

      if (a == hack->position ) {
        int j;
        for ( j=0; j<hack->size; j++) {
          mvprintw( terminalStartY + arrayToPoint(a).y, terminalStartX + arrayToPoint(a).x, "%c", *(boardPtr + a) );
          a++;
        }
      }
    }

    attroff(A_STANDOUT);
    return;
  }

  printw("%c", *(boardPtr + a));
  attroff(A_STANDOUT);
}

char *stringAtCursor() {
  char * boardPtr = getBoard();
  PasswordPtr * hacksPtr = getHacks();

  if (insideWord() >= 0) {  /* Cursor is in word. */
    char *check = malloc(sizeof(char) * getPasswordLength());

    int k = insideWord();

    int i;
    for ( i=0; i< getPasswordLength(); i++ ) {
      check[i] = *(boardPtr + k++);
    };

    return check;
  } else if (insideHack() >= 0) {

    /* Find the hack that we are inside. */
    int hackPosition = insideHack();

    int i;
    for ( i = 0; i < getNumHacks(); i++ ) {
      PasswordPtr hack = *(hacksPtr + i);
      if ( hack->position == hackPosition ) {
        return hack->content;
      }
    }
  }

  /* Cursor is on a single char. */
  char *returner = malloc(sizeof(char) * 2);
  returner[0] = *(boardPtr + arrayFromYX(cur.y, cur.x));
  returner[1] = '\0';
  return returner;
}

int insideWord() {  /* if inside word, return array start position, else -1 */
  PasswordPtr * passwordsPtr = getPasswords();

  int a = arrayFromYX(cur.y, cur.x);

  int i;
  for (i = 0; i < getNumPasswords(); i++) {  /* iterate through all words */
    PasswordPtr password = *(passwordsPtr + i);
    if (a >= password->position && a < password->position + getPasswordLength()) {
      return password->position;
    }
  }

  return -1;
}

int insideHack() {  /* if inside word, return array start position, else -1 */
  PasswordPtr * hacksPtr = getHacks();

  int a = arrayFromYX(cur.y, cur.x);

  int i;
  for (i = 0; i < getNumHacks(); i++) {  /* iterate through all words */
    PasswordPtr hack = *(hacksPtr + i);
    if (a == hack->position )
      return hack->position;
  }

  return -1;
}

void getUserInput() {
  updateAttemptsLeft();

  printInputArea();

  if ( getLoggedInStatus() == TRUE ) {
    usleep(TYPE_SPEED*3);
    accessSystem();
  }

  int uInput = getch();  /* Get user input from keyboard (pause) */
  usleep(10);  /* Reduces cursor jump if arrows are held down */

  switch (uInput) {
    case '\n' :
      tryPassword();
      break;
    case KEY_UP :
      cursorUp();
      break;
    case KEY_DOWN :
      cursorDown();
      break;
    case KEY_LEFT :
      cursorLeft();
      break;
    case KEY_RIGHT :
      cursorRight();
      break;
      //TODO: These are disabled for now.
//      case '-' :
//        decrementTriesLeft();
//        break;
//      case '+' :
//        if (triesLeft < 4) {
//          triesLeft++;
//          pushMessage("Allowance");
//          pushMessage("replenished.");
//        }
//        break;
//      case 'a' :
//        loggedIn = TRUE;
//        break;
    case 'q' :  /* Quit key */
      exitUos( EXIT_SUCCESS );
    default:
      break;
  }
}

void cursorUp() {
  if (cur.y != 6) {
    cur.y--;
  }
}

void cursorDown() {
  if (cur.y != 22) {
    cur.y++;
  }
}

void cursorLeft() {
  if (insideWord() > -1 )                /* If the cursor is inside a word-                            */
    cur.x = calculateNextCurX( KEY_LEFT );  /*   figure out the next cur.x location.                      */
  else if (cur.x == 28)                   /* Else if the cursor is at the left bound of the right side- */
    cur.x = 19;                           /*   move the cursor to the right bound of the left side.     */
  else if (cur.x != 8)                    /* As long as the cursor is not at the far left of the board- */
    cur.x--;                              /*   move the cursor left.                                    */
}

void cursorRight() {
  if (insideWord() > -1 )                /* If the cursor is inside a word-                             */
    cur.x = calculateNextCurX( KEY_RIGHT );  /*   figure out the next cur.x location.                       */
  else if (cur.x == 19)                   /* Else if the cursor is at the right bound of the left side-  */
    cur.x = 28;                           /*   move the cursor to the left bound of the right side.      */
  else if (cur.x != 39)                   /* As long as the cursor is not at the far right of the board- */
    cur.x++;                              /*   move the cursor right.                                    */
}

int calculateNextCurX( int keyDirection ) {
  int startOfWord = insideWord();
  int wordStartLine = startOfWord /12;
  int currentArrayPosition = arrayFromYX(cur.y, cur.x);
  int returnVal;

  // TODO: Refactor the code below. There must be a better way!
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
        if ( wordStartLine != (startOfWord + getPasswordLength())/12)
          returnVal = 28;
        else
          returnVal = arrayToPoint(insideWord()).x + getPasswordLength();
      } else {                                                          /* Cursor is not on same line as start of word. */
        returnVal = START_X + getPasswordLength() - (12 - ( startOfWord - ( wordStartLine * 12 ) ) );
      }
    } else {                                                            /* Cursor is on the right side of the board. */
      if ( currentArrayPosition /12 == wordStartLine) {                 /* Cursor is on same line as start of word. */
        if ( wordStartLine == (startOfWord + getPasswordLength())/12)
          returnVal = arrayToPoint(insideWord()).x + getPasswordLength();
        else
          returnVal = cur.x;
      } else {                                                          /* Cursor is not on same line as start of word. */
        returnVal = START_X + 20 + getPasswordLength() - (12 - ( startOfWord - ( wordStartLine * 12 ) ) );
      }
    }
  }

  return returnVal;
}
