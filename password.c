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

PasswordPtr createHack( const char *str ) {
	PasswordPtr newHack = (PasswordPtr) malloc( sizeof( Password ) );
	newHack->size = strlen(str);
	newHack->content = malloc( sizeof(char) * (newHack->size + 1) );
	newHack->removed = FALSE;
	strcpy(newHack->content, str);
	
	return newHack;
}

void setHackPosition( PasswordPtr hack, int newposition) {
	hack->position = newposition;
}

void freePassword( PasswordPtr password) {
	free(password->content);
	free(password);
}