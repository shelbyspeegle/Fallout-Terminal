#ifndef __PASSWORD_H
#define __PASSWORD_H

#include <stdlib.h>
#include <string.h>
#include "common.h"

typedef struct password Password;
typedef struct password *PasswordPtr;

struct password {
  char *content;
  int size;
  int position;
  boolean correct;
  boolean removed;
};

/**
 *  Allocates space for a new Password and its contents.
 */
PasswordPtr createPassword( const char *str, int position );

/**
 *  Allocates space for a new hack and its contents. Generates a hack of random
 *    length.
 */
PasswordPtr createHack();

/**
 *  Set the position of a hack.
 */
void setHackPosition( PasswordPtr hack, int position );

/**
*   Compares the bracket type of two different hacks. Returns true if they match.
*/
boolean sameBracketType( PasswordPtr hack_a, PasswordPtr hack_b );

/**
 *  Frees the Password parameter along with its contents.
 */
void freePassword( PasswordPtr password );

#endif /* password.h */