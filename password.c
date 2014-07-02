#include "password.h"

PasswordPtr createPassword( const char *str, int position) {
	PasswordPtr newPassword = (PasswordPtr) malloc( sizeof( Password ) );
	newPassword->size = strlen(str);
	newPassword->position = position;
	newPassword->correct = FALSE;
	newPassword->content = (char *) str;
	
	return newPassword;
}

void freePassword( PasswordPtr password) {
	free(password->content);
	free(password);
}