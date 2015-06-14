#include "game_controller.h"

#include "game_state.h"
#include "interface.h"

void run() {
  initializeInterface();

  if ( terminalTooSmall() ) {
    error_terminalTooSmall();
    exit( EXIT_ERROR );
  }

  wordList = malloc( sizeof(char *) * 40 );

  loadFiles();

  printScreen1();
  printScreen2();
  printScreen3();

  setup();

  while (1) {  /* Start the game. */
    getUserInput();
  }
}

int getDebugStatus() {
  return debug;
}

void setDebugStatus( int newDebugStatus ) {
  debug = newDebugStatus;
}

int getHardmodeStatus() {
  return HARDMODE;
}

int getLoggedInStatus() {
  return loggedIn;
}

void setAttemptsLeft( int newAttemptsLeft ) {
  attemptsLeft = newAttemptsLeft;
}
int getAttemptsLeft() {
  return attemptsLeft;
}

char * getBoard() {
  return board;
}

char ** getMessages() {
  return messages;
}

PasswordPtr * getHacks() {
  return hacks;
}

PasswordPtr * getPasswords() {
  return passwords;
}

int getNumHacks() {
  return NUM_HACKS;
}

int getNumPasswords() {
  return NUM_PASSWORDS;
}

int getPasswordLength() {
  return PASSWORD_LENGTH;
}

int getMaxNumMessages() {
  return MAX_MESSAGES;
}

void tryPassword() {
  if (insideWord() >= 0) {  /* Test if the cursor is on a Password. */
    int k = insideWord();  /* Set k to the start of the word the cursor is in. */
    char *check = malloc(sizeof(char) * PASSWORD_LENGTH);
    int j;
    for (j=0; j< PASSWORD_LENGTH; j++) {
      check[j] = board[k++];
    };

    pushMessage( stringAtCursor() );

    int i = numberOfCorrectChars(check);
    if (i == PASSWORD_LENGTH) {
      pushMessage("Exact match!");
      pushMessage("Please wait");
      pushMessage("while system");
      pushMessage("is accessed.");

      loggedIn = TRUE;
    } else {
      pushMessage("Entry denied");
      char *stringBuild = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
      sprintf(stringBuild, "%i/%i correct.", i, PASSWORD_LENGTH);
      pushMessage(stringBuild);

      setAttemptsLeft( getAttemptsLeft() - 1 );
    }
  } else if ( insideHack() >= 0 ) {  /* Test if the cursor on a Hack. */

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

    char *stringBuild = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
    sprintf(stringBuild, "0/%i correct.", PASSWORD_LENGTH);
    pushMessage(stringBuild);
    setAttemptsLeft( getAttemptsLeft() - 1 );
  }

  loggedIn = FALSE;
}

void accessSystem() {
  useconds_t fastPrintSpeed = PRINT_SPEED/3;

  clear();

  mvtermprint( terminalStartY + 1, terminalStartX + 1, "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK", fastPrintSpeed );
  mvtermprint( terminalStartY + 3, terminalStartX + 1, " LOGIN ADMIN", fastPrintSpeed );
  mvtermprint( terminalStartY + 5, terminalStartX + 1, "ENTER PASSWORD NOW", fastPrintSpeed );

  mvprintw( terminalStartY + 7, terminalStartX + 1, ">" );
  refresh();
  usleep(TYPE_DELAY);  /* TODO: get exact time */
  int i;
  for (i=0; i < PASSWORD_LENGTH; i++) {
    mvprintw( terminalStartY + 7, terminalStartX + 3+i, "*" );
    usleep( TYPE_SPEED*3 );
    refresh();
  }
  usleep(TYPE_DELAY);  /* TODO: get exact time */

  clear();

  mvtermprint( terminalStartY + 1, terminalStartX + 7, "ROBCO INDUSTRIES UNIFIED OPERATING SYSTEM", fastPrintSpeed );
  mvtermprint( terminalStartY + 2, terminalStartX + 9, "COPYRIGHT 2075-2077 ROBCO INDUSTRIES", fastPrintSpeed );
  mvtermprint( terminalStartY + 3, terminalStartX + 22, "-Server 1-", fastPrintSpeed );

  /* If Lock ///////////////////////////////////////////////////////////////*/
  mvtermprint( terminalStartY + 5, terminalStartX + 9, "SoftLock Solutions, Inc", fastPrintSpeed );
  mvtermprint( terminalStartY + 6, terminalStartX + 5, "\"Your Security is Our Security\"", fastPrintSpeed );
  mvtermprint( terminalStartY + 7, terminalStartX + 5, ">\\ Welcome, USER", fastPrintSpeed );
  mvtermprint( terminalStartY + 8, terminalStartX + 5, "_______________________________", fastPrintSpeed );

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
        mvprintw( terminalStartY + 9+j, terminalStartX + 5, "                                            " );
        mvprintw( terminalStartY + 9+j, terminalStartX + 5, "> %s", menu[j] );
        attroff(A_STANDOUT);
      } else {
        mvprintw( terminalStartY + 9+j, terminalStartX + 5, "                                            " );
        mvprintw( terminalStartY + 9+j, terminalStartX + 5, "> %s", menu[j] );
      }
    }

    mvprintw( terminalStartY + 22, terminalStartX + 5, "                                        " );
    mvprintw( terminalStartY + 22, terminalStartX + 5, "> " );

    int uInput = getch();  /* Get user input from keyboard (pause). */
    usleep(10);  /* Reduces cursor jump if arrows are held down. */

    switch (uInput) {
      case '\n' :           /* TODO: test Linux */
        mvtermprint( terminalStartY + 22, terminalStartX + 7, menu[selection], PRINT_SPEED );
        if (getch() == '\n') {
          mvprintw( terminalStartY + 22, terminalStartX + 5, "                                        " );
          mvprintw( terminalStartY + 22, terminalStartX + 5, "> " );
          mvtermprint( terminalStartY + 22, terminalStartX + 7, "Exiting...", PRINT_SPEED );
          usleep(TYPE_DELAY*3);
          exitUos( EXIT_SUCCESS );
        }
        break;
      case KEY_UP :
        selection == 0 ? selection = 1 : selection--;
        break;
      case KEY_DOWN :
        selection == numOptions - 1 ? selection = 0 : selection++;
        break;
      case 'q' :  /* Quit key */
        exitUos( EXIT_SUCCESS );
      default:
        break;
    }
  }
}



void exitUos( int exitStatus ) {
  free( hacks );
  free( messages );

  endwin();  /* End ncurses mode */
  exit( exitStatus );
}

void loadFiles() {
  /* Word lengths between 4-15 */
  char *fileName = (char *) malloc( sizeof(char) * 16 );
  sprintf(fileName, "wordbank/%i.txt", PASSWORD_LENGTH);

  fr = fopen(fileName, "r");

  int n = PASSWORD_LENGTH + 2;
  char line[n];

  int m = 0;
  for ( m=0; m<40; m++ ) {
    fgets( line, n, fr );
    wordList[m] = malloc( sizeof(char) * PASSWORD_LENGTH);
    strncpy( wordList[m], line, PASSWORD_LENGTH);
  }

  free(fileName);
  fclose(fr);
}