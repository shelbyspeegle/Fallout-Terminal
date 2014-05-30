/*
 * f3terminal.c
 *
 *  Created on: May 3, 2014
 *      Author: admin
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>

#define TYPEDELAY 40000
#define PRNTDELAY 20000
#define LINEDELAY 15000

void termPrint(char *s);
void termType(char *s);
void termClr();
void updateScreen();
static void finish(int sig);

int trysLeft = 4;

int main(int argc, char *argv[]) {

	termClr();

	char *s1 = "SECURITY RESET...\r";
	char *s2 = "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK\n";
	char *t1 = ">SET TERMINAL/INQUIRE\n";
	char *s3 = "RIT-V300\n";

	char *t2 = ">SET FILE/PROTECTION=OWNER:RWED ACCOUNTS.F\n";

	termPrint(s1);
	termClr();
	termPrint(s2);
	termType(t1);
	termPrint(s3);
	termType(t2);

	char s[81]; // string of 80 chars + '\0'	

	int going = 1;	

	while (going) {

		updateScreen();
		
		if (fgets(s, 81, stdin))
			; // hack to silence fgets

		if (s[0] == 'q') {
			going = 0;
		} else {
			trysLeft--;
			if (trysLeft == 0)
				going = 0;
		}
	}	

	finish(0);

	return(0);
}

void termPrint(char *s) {
	printf("\n");
	int i;
	for (i = 0; i < strlen(s) + 1; i++) {
		fflush(stdout);
		printf("%c", s[i]);
		usleep(PRNTDELAY);
	}
	usleep(LINEDELAY);
}

void termType(char *s) {
	printf("\n");
	int i;
	for (i = 0; i < strlen(s) + 1; i++) {
		fflush(stdout);
		printf("%c", s[i]);
		usleep(TYPEDELAY);
	}
	usleep(LINEDELAY);
}

void termClr() {
	if(system("clear"))
		; // hack to silence compiler
}

void updateScreen() {
	termClr();
	char *s4 = "ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL\nENTER PASSWORD NOW\n\n";
	printf("%s", s4);

	printf("%i ATTEMPT(S) LEFT :", trysLeft);

	int i;
	for (i = 0; i < trysLeft; i++) {
		printf(" █");
	}
	printf("\n\n");

	// PUZZLE CONTENT //////////////////////////////////////////////////////////////////

	printf("0xF92C ,\";(:}(=DANG 0xF9F8 \\_'*_)^,@_%%$\n");
	printf("0xF938 ERS)'!?\"<^%%< 0xFA04 \\_'*_)^,@_%%$\n");
	printf("0xF944 _).!]\\./+)\"| 0xFA10 \\_'*_)^,@_%%$\n");
	printf("0xF950 ,\";(:}(=DANG 0xFA1C \\_'*_)^,@_%%$\n");
	printf("0xF95C ,\";(:}(=DANG 0xFA28 \\_'*_)^,@_%%$\n");
	printf("0xF968 ,\";(:}(=DANG 0xFA34 \\_'*_)^,@_%%$\n");
	printf("0xF974 ,\";(:}(=DANG 0xFA40 \\_'*_)^,@_%%$\n");
	printf("0xF980 ,\";(:}(=DANG 0xFA4C \\_'*_)^,@_%%$\n");
	printf("0xF98C ,\";(:}(=DANG 0xFA58 \\_'*_)^,@_%%$\n");
	printf("0xF998 ,\";(:}(=DANG 0xFA64 \\_'*_)^,@_%%$\n");
	printf("0xF9A4 ,\";(:}(=DANG 0xFA70 \\_'*_)^,@_%%$\n");
	printf("0xF9B0 ,\";(:}(=DANG 0xFA7C \\_'*_)^,@_%%$\n");
	printf("0xF9BC ,\";(:}(=DANG 0xFA88 \\_'*_)^,@_%%$ "); printf(">CISTERN\n");
	printf("0xF9C8 ,\";(:}(=DANG 0xFA94 \\_'*_)^,@_%%$ "); printf(">Entry denied\n");
	printf("0xF9D4 ,\";(:}(=DANG 0xFAA0 \\_'*_)^,@_%%$ "); printf(">0/7 correct.\n");
	printf("0xF9E0 ,\";(:}(=DANG 0xFAAC \\_'*_)^,@_%%$\n");
	printf("0xF9EC ,\";(:}(=DANG 0xFAB8 \\_'*_)^,@_%%$ ");

	printf(">");
}

static void finish(int sig) {
	char ch;
	if (sig == 0) {

		termClr();
        termPrint("LOCKED OUT\nPLEASE CONTACT AN ADMINISTRATOR\n\nPRESS ENTER TO CONTINUE.\n");

		if(scanf("%c",&ch))
			; // hack to silence compiler
		termClr();
	}

	termClr();
}
