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
};

/**
 *	Allocates space for a new Password and its contents.
 */
PasswordPtr createPassword( const char *str, int position);

/**
 *	Frees the Password parameter along with its contents.
 */
void freePassword(PasswordPtr password);

#endif /* password.h */