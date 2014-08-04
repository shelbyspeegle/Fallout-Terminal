#include "password.h"
#include "utilities.h"

PasswordPtr createPassword( const char *str, int position) {
  PasswordPtr newPassword = (PasswordPtr) malloc( sizeof( Password ) );
  newPassword->size = strlen(str);
  newPassword->position = position;
  newPassword->correct = FALSE;
  newPassword->content = malloc( sizeof(char) * (newPassword->size + 1) );
  newPassword->removed = FALSE;
  strcpy(newPassword->content, str);

  return newPassword;
}

PasswordPtr createHack() {
  PasswordPtr newHack = (PasswordPtr) malloc( sizeof( Password ) );

  /* Pick a hack length from 2 - 11 */
  newHack->size = ( rand() % 10 ) + 2;

  newHack->content = malloc( sizeof(char) * (newHack->size + 1) );
  newHack->removed = FALSE;

  char openbrackettype;
  char closingbrackettype;

  /* Pick type of bracket to use for hack. */
  switch (rand() % 4) { /* Pick number from 0 - 2 */
    case 0:
      openbrackettype = '(';
      closingbrackettype = ')';
      break;
    case 1:
      openbrackettype = '[';
      closingbrackettype = ']';
      break;
    case 2:
      openbrackettype = '{';
      closingbrackettype = '}';
      break;
  }

  newHack->content[0] = openbrackettype;
  newHack->content[newHack->size-1] = closingbrackettype;

  /* Fill hack contents with trash (except brackets of type above) */
  int i;
  for ( i=1; i<newHack->size-1; i++ ) {
    char trash;
    do {
      trash = genTrash();
    } while ( trash == openbrackettype || trash == closingbrackettype );

    newHack->content[i] = trash;
  }

  return newHack;
}

void setHackPosition( PasswordPtr hack, int newposition) {
  hack->position = newposition;
}

void freePassword( PasswordPtr password) {
  free(password->content);
  free(password);
}