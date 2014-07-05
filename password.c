#include "password.h"

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

void freePassword( PasswordPtr password) {
	free(password->content);
	free(password);
}