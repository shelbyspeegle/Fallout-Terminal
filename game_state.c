#include "game_state.h"

#include "game_controller.h"

boolean HARDMODE = FALSE;
int PASSWORD_LENGTH = 9;
boolean debug = FALSE;
boolean loggedIn = FALSE;
int passwordsLeftOnBoard = NUM_PASSWORDS;

int attemptsLeft = 4;

void pushMessage( const char *newMessage ) {
  char *fullMsg = malloc(sizeof(char) * MAX_MESSAGE_LENGTH + 1);

  /* Start the message off with a '>' */
  fullMsg[0] = '>';

  /* Copy all of the contents of the passed in string. */
  int i;
  for ( i=1; i<strlen(newMessage)+1; i++ ) {
    fullMsg[i] = newMessage[i-1];
  }

  /* Fill the rest of the string with spaces. */
  while (i < MAX_MESSAGE_LENGTH) {
    fullMsg[i++] = ' ';
  }

  int j;
  for ( j=0; j < MAX_MESSAGES; j++ ) {
    messages[MAX_MESSAGES-j] = messages[MAX_MESSAGES-j-1];
  }
  messages[0] = fullMsg;
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

  int max = 407-1- PASSWORD_LENGTH;
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
        if ( positionForNewPassword < newPasswordPositions[j] + PASSWORD_LENGTH + 1
            && positionForNewPassword + PASSWORD_LENGTH > newPasswordPositions[j]) {
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

char *uniqueRandomWord() {
  /* TODO: Make each work unique. */

  /* Pick a word from 0 - 39 */
  return wordList[ rand() % 40 ];
}

int numberOfCorrectChars( const char *checkWord ) {
  if (strlen(checkWord) != PASSWORD_LENGTH) {
    /* TODO: properly free memory and end the program */
    fprintf(stderr, "ERROR: numberOfCorrectChars called on string of length %i when passwords are of length %i.\n",
        (int) strlen(checkWord), PASSWORD_LENGTH);
    exitUos( EXIT_ERROR );
  }

  int count = 0;

  int i;
  for ( i=0; i < PASSWORD_LENGTH; i++ ) {
    if ( passwords[correct]->content[i] == checkWord[i] ) {
      count++;
    }
  }

  return count;
}

void setup() {
  messages = malloc( sizeof(char*) * MAX_MESSAGES);
  hacks = malloc( sizeof(char*) * NUM_HACKS);
  passwords = malloc( sizeof(PasswordPtr *) * NUM_PASSWORDS );

  /* Populate Board Array with Trash /////////////////////////////////////////*/
  int i;
  for (i = 0; i < 408; i++) {
    board[i] = genTrash();
  }

  genPasswords();
  refreshPasswords();
  genHacks();
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