/*
 * hackterm.c
 *
 *  Created on: May 3, 2014
 *      Author: shelbyspeegle
 */

#include <ncurses.h>						/* ncurses.h includes stdio.h */
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h> 							/* for srand(time(NULL)) */

#define MAX_MESSAGES 15
#define MAX_MESSAGE_LENGTH 13
#define NUM_PASSWORDS 10 					/* i think it is 17 */
#define NUM_HACKS 5 						/* TODO: find the real number */
#define START_Y 6
#define START_X 8

typedef int boolean;
#define TRUE 1
#define FALSE 0

struct point {
	int y;
	int x;
};
typedef struct point Point;

/*
 Difficulty	Length
 Very Easy	4-5
 Easy			6-8
 Average		9-10
 Hard			11-12
 Very Hard	13-15
*/

char *registers[] = {
	"0xF964", "0xF970", "0xF97C", "0xF988", "0xF994", "0xF9A0", "0xF9AC",
	"0xF9B8", "0xF9C4", "0xF9D0", "0xF9DC", "0xF9E8", "0xF9F4", "0xFA00",
	"0xFA0C", "0xFA18", "0xFA24",

	/* Repeat of the first 17 TODO: replace with real registers */
	"0xF964", "0xF970", "0xF97C", "0xF988", "0xF994", "0xF9A0", "0xF9AC", 
	"0xF9B8", "0xF9C4", "0xF9D0", "0xF9DC", "0xF9E8", "0xF9F4", "0xFA00", 
	"0xFA0C", "0xFA18", "0xFA24"
};

char **messages; 				/* Array of message strings. */
char board[408]; 				/* Board of 408 chars. */
int hackLocations[NUM_HACKS]; 	/* Series of array positions where hacks are */
int passLocations[] = {0,21,55,79,111,175,220,270,300,390}; /*TODO: make dynamic */
int passwordLength = 8; 		/* TODO: make dynamic */
char **hacks;
char **passwords;
int rows, cols;
int trysLeft = 4;
int correct;	/* The index position of the right password in passLocations[] */
Point cur;
int TYPE_SPEED = 24000; /* TODO: make these constants when program is finished */
int PRINT_SPEED = 18000;

void setup();
void printinputarea();
void refreshboard();
void pushmessage(const char *newMessage);
boolean tryPassword();
void removeDud(int a);
void addPasswordsToBoard();
int yxtoarray(int y, int x);
Point arraytopoint(int a);
char genTrash();
void genPasswords();
int insideWord();
void highlight();
char *stringatcursor();
void mvtermprint(int y, int x, char *string, int speed);
int numberofcorrectchars(const char *checkword);
void accesssystem();
void exituos();
void lockterminal();

int main(int argc, char **argv) {
	
	boolean debug = FALSE;
	boolean hardmode = FALSE;
	
	srand(time(0)); /* Seed rand with this so it is more random */
	
	initscr();						/* Start ncurses mode */
	getmaxyx(stdscr,rows,cols);		/* Get the number of rows and columns */
	keypad(stdscr, TRUE);			/* Converts arrow key input to usable chars */

	if (rows < 24 || cols < 55) {	/* Check to see if window is big enough */
		
		/*TODO: Make terminal centered at all resolutions */
		
		endwin();
		printf("ERROR: Terminal window is too small,\n");
		printf("       minimum size of 24x55 to run.\n");
		return 0;
		
		/* 53 for play area, +1 on each side for padding */
	}
	
	/* Cut to the chase when debugging */
	if ( debug ) TYPE_SPEED = PRINT_SPEED = 0;
	
	/* Start of display */
	mvtermprint( 1, 1, "SECURITY RESET...", PRINT_SPEED);
	clear();
	mvtermprint( 3, 1, "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK", PRINT_SPEED);

	if ( hardmode ) { /* Manual input mode */

		int curLine = 5;

		mvprintw( curLine, 1, ">") ;

		char str[80]; /* TODO: Find number + enforce */

		boolean instartup = TRUE;

		while ( instartup ) {
			
			getstr( str );

			if ( strcmp( str, "SET TERMINAL/INQUIRE" ) == 0 ) {
				curLine += 2;
				mvprintw( curLine, 1, "RIT-V300" );
				curLine += 2;
			} else if ( strcmp( str, "SET FILE/PROTECTION=OWNER:RWED ACCOUNTS.F" ) == 0) {
				curLine++;
			} else if ( strcmp( str, "SET HALT RESTART/MAINT" ) == 0 ) {
				curLine += 2;
				mvtermprint( curLine++, 1, "Initializing Robco Industries(TM) MF Boot Agent v2.3.0", PRINT_SPEED);
				mvtermprint( curLine++, 1, "RETROS BIOS", PRINT_SPEED);
				mvtermprint( curLine++, 1, "RBIOS-4.02.08.00 52EE5.E7.E8", PRINT_SPEED);
				mvtermprint( curLine++, 1, "Copyright 2201-2203 Robco Ind.", PRINT_SPEED);
				mvtermprint( curLine++, 1, "Uppermem: 64 KB", PRINT_SPEED);
				mvtermprint( curLine++, 1, "Root (5A8)", PRINT_SPEED);
				mvtermprint( curLine, 1, "Maintenance Mode", PRINT_SPEED);
				curLine += 2;
			} else if ( strcmp( str, "RUN DEBUG/ACCOUNTS.F" ) == 0 ) {
				instartup = FALSE;
			} else if ( strcmp( str, "EXIT" ) == 0 ) {
				curLine += 2;
				mvtermprint( curLine, 1, "EXITING...", PRINT_SPEED );
				refresh();
				usleep(1000000);
				exituos();
			} else {
				curLine += 2;

				char builder[80];
				builder[0] = 'U';
				builder[1] = 'O';
				builder[2] = 'S';
				builder[3] = ':';
				builder[4] = ' ';

				int count = 5;

				while ( (builder[count] = str[count-5]) ) {
					count++;
				}
				builder[count++] = ':';
				builder[count++] = ' ';
				builder[count++] = 'c';
				builder[count++] = 'o';
				builder[count++] = 'm';
				builder[count++] = 'm';
				builder[count++] = 'a';
				builder[count++] = 'n';
				builder[count++] = 'd';
				builder[count++] = ' ';
				builder[count++] = 'n';
				builder[count++] = 'o';
				builder[count++] = 't';
				builder[count++] = ' ';
				builder[count++] = 'f';
				builder[count++] = 'o';
				builder[count++] = 'u';
				builder[count++] = 'n';
				builder[count++] = 'd';

				mvprintw( curLine, 1, "%s", builder );
				curLine += 2;
			}

			mvprintw(curLine, 1, ">");
		}
	} else {
		mvtermprint( 5, 1, "SET TERMINAL/INQUIRE", TYPE_SPEED);

		mvtermprint( 7, 1, "RIT-V300", PRINT_SPEED);

		mvtermprint( 9, 1, "SET FILE/PROTECTION=OWNER:RWED ACCOUNTS.F", TYPE_SPEED);

		mvtermprint( 10, 1, "SET HALT RESTART/MAINT", TYPE_SPEED);

		mvtermprint( 12, 1, "Initializing Robco Industries(TM) MF Boot Agent v2.3.0", PRINT_SPEED);
		mvtermprint( 13, 1, "RETROS BIOS", PRINT_SPEED);
		mvtermprint( 14, 1, "RBIOS-4.02.08.00 52EE5.E7.E8", PRINT_SPEED);
		mvtermprint( 15, 1, "Copyright 2201-2203 Robco Ind.", PRINT_SPEED);
		mvtermprint( 16, 1, "Uppermem: 64 KB", PRINT_SPEED);
		mvtermprint( 17, 1, "Root (5A8)", PRINT_SPEED);
		mvtermprint( 18, 1, "Maintenance Mode", PRINT_SPEED);
		
		mvtermprint( 20, 1, "RUN DEBUG/ACCOUNTS.F", TYPE_SPEED);
	}
	
	clear();
	
	noecho();						/* Silence user input */
	
	setup();
	
	boolean loggedin = FALSE;
	
	while (1) {
		/* Update the attempts left section //////////////////////////////////*/
		mvprintw( 4, 21, "        "); 					/* Clear the tries */
		
		if (trysLeft == 1) {
			attron(A_BLINK); /* eh blinkin'! */
			mvprintw(2, 1, "!!! WARNING: LOCKOUT IMMINENT !!!");
			attroff(A_BLINK);
		} else {
			mvprintw(2, 1, "ENTER PASSWORD NOW               ");
		}
		
		/* Place the four symbols for tries left */
		attron(A_STANDOUT);
		switch (trysLeft) {
			case 4:
				mvprintw( 4, 27, " "); /* 4th */
			case 3:
				mvprintw( 4, 25, " "); /* 3rd */
			case 2:
				mvprintw( 4, 23, " "); /* 2nd */
			case 1: /* Lockout imminent */
				mvprintw( 4, 21, " "); /* 1st */
				attroff(A_STANDOUT);
				break;
			default: /* Game over */
				attroff(A_STANDOUT);
				lockterminal();
		}
		
		mvprintw(4, 1, "%i", trysLeft);
		
		refreshboard();
		
		move(cur.y, cur.x);
		
		highlight();
		refresh();
		
		printinputarea();

		if (loggedin) {
			usleep(1000000*3);
			accesssystem();
		}

		int uInput = getch();	/* Get user input from keyboard (pause) */
		usleep(10); 			/* Reduces cursor jump if arrows are held down */
		
		switch (uInput) {
		case '\n' :				/* TODO: test Linux */
			loggedin = tryPassword();
			break;
		case KEY_UP :
			if (cur.y != 6)
				cur.y--;
			break;
		case KEY_LEFT :
			if (cur.x == 28)
				cur.x = 19;
			else if (cur.x != 8)
				cur.x--;
			break;
		case KEY_DOWN :
			if (cur.y != 22)
				cur.y++;
			break;
		case KEY_RIGHT :
			if (cur.x == 19)
				cur.x = 28;
			else if (cur.x != 39)
				cur.x++;
			break;
		case 'e' :
			removeDud(0);
			break;
		case '-' :
			trysLeft--;
			break;
		case '+' :
			if (trysLeft < 4) {
				trysLeft++;
				pushmessage("Allowance");
				pushmessage("replenished.");
			}
			break;
		case 'a' :
			loggedin = TRUE;
			break;
		case 'q' : /* Quit key */
			exituos();
		default:
			break;
		}
		
		/* 
		if inside a word
			if key right
				if not at MAX, go to arraystartposition + WORDLENGTH + 1
			if key left
				if not at 0, go to arraystartposition of word - 1
		*/
	}
	exituos();
}

void setup() {
	/* Board Set-up //////////////////////////////////////////////////////////*/
	mvprintw( 22, 41, ">" );
	refresh();
	
	/* Place cursor at starting position (board[0]) */
	cur.y = START_Y;
	cur.x = START_X;
	
	messages = malloc( sizeof(char*) * MAX_MESSAGES);
	hacks = malloc( sizeof(char*) * NUM_HACKS);
	passwords = malloc( sizeof(char*) * NUM_PASSWORDS );
	mvtermprint( 1, 1, "ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL", PRINT_SPEED );
	mvtermprint( 2, 1, "ENTER PASSWORD NOW", PRINT_SPEED );
	mvtermprint( 4, 1, "4 ATTEMPT(S) LEFT : ", PRINT_SPEED );
	
	attron(A_STANDOUT);
	mvprintw(4, 21, " "); /* Print 1st */
	usleep(PRINT_SPEED*2);
	mvprintw(4, 23, " "); /* Print 2nd */
	usleep(PRINT_SPEED*2);
	mvprintw(4, 25, " "); /* Print 3rd */
	usleep(PRINT_SPEED*2);
	mvprintw(4, 27, " "); /* Print 4th */
	usleep(PRINT_SPEED);
	attroff(A_STANDOUT);

	/* Populate Board Array with Trash ///////////////////////////////////////*/
	int i;
	for (i = 0; i < 408; i++) {
		board[i] = genTrash();
	}
	
	genPasswords();

	/* TODO: clean up the code below, it is pretty messy */
	/* Print Registers ///////////////////////////////////////////////////////*/
	
	int printX = 1;
	int printY = 6;
	
	/* Print left half of game */
	for (i = 0; i < 17; i++) {
		mvtermprint(printY, printX, registers[i], PRINT_SPEED/6 );
		printX+=7;
		refresh();
		
		int j;
		for (j = 0; j < 12; j++) { /* iterate through each char in array */
			mvprintw(printY, printX++, "%c", board[ j + (12 * i) ]);
			usleep(PRINT_SPEED/6);
			refresh();
		}
		
		printX = 1;
		printY++;
	}
	
	printY = 6;
	printX = 21;

	/* Print right half of game */
	for (i = 0; i < 17; i++) {
		mvtermprint(printY, printX, registers[i+17], PRINT_SPEED/6 );
		printX+=7;
		refresh();
		
		int j;
		for (j = 0; j < 12; j++) { /* iterate through each char in array */
			mvprintw(printY, printX++, "%c", board[204 + j + (12 * i) ]);
			usleep(PRINT_SPEED/6);
			refresh();
		}
		
		printX = 21;
		printY++;
	}
	
}

void printinputarea() {
	int i;
	for (i=0; i < MAX_MESSAGES; i++) {
		if (messages[i])
			mvprintw(20-i, 41, "%s", messages[i]); /* Print messages that exist */
	}
	
	mvprintw( 22, 42, "              ");				/* Clear input line */
	
	mvtermprint( 22, 42, stringatcursor(), PRINT_SPEED );
}

void refreshboard() {
	int i;
	for (i = 0; i < 408; i++) { /* iterate through each char in array */
		mvprintw(arraytopoint(i).y, arraytopoint(i).x, "%c", board[i]);
	}
}

void pushmessage(const char *newMessage) {
	char *fullMsg = malloc(sizeof(char) * MAX_MESSAGE_LENGTH + 1);
	
	/* Start the message off with a '>' */
	fullMsg[0] = '>';
	
	/* Copy all of the contents of the passed in string */
	int k;
	for (k=1; k<strlen(newMessage)+1; k++) {
		fullMsg[k] = newMessage[k-1];
	}
	
	/* Fill the rest of the string with spaces */
	while (k < MAX_MESSAGE_LENGTH) {
		fullMsg[k++] = ' ';
	}
	
	int i;
	for (i=0; i < MAX_MESSAGES; i++) {
		messages[MAX_MESSAGES-i] = messages[MAX_MESSAGES-i-1];
	}
	messages[0] = fullMsg;
}

boolean tryPassword() {
	if (insideWord() >= 0) {
		int k = insideWord(); /* start of the word */
		char *check = malloc(sizeof(char) * passwordLength);
		int j;
		for (j=0; j<passwordLength; j++) {
			check[j] = board[k++];
		};
		
		pushmessage( stringatcursor() );

		int i = numberofcorrectchars(check);
		if (i == passwordLength) {
			pushmessage( "Exact match!" );
			pushmessage( "Please wait" );
			pushmessage( "while system" );
			pushmessage( "is accessed." );
			
			/*free(check) */

			return TRUE;
		} else {
			pushmessage( "Entry denied" );
			char *stringbuild = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
			sprintf(stringbuild, "%i/%i correct.", i, passwordLength);
			pushmessage( stringbuild );
			trysLeft--;
		}
	}
	/* TODO: test for hacks and trash */
	
	/*
	else if (insideHack() >= 0) {
		pushmessage(hackContents);
		pushmessage("Dud removed.");
		prints 2 lines
		or
		pushmessage(hackContents);
	 pushmessage("Allowance");
		pushmessage("replenished.");
		}
	If trash
		TODO: what does trash do?
	*/
	
	return FALSE;
}

void removeDud(int a) { /* TODO: unimplemented. */
	pushmessage(">Dud removed.");
	pushmessage(">[*(>]");
}

int yxtoarray(int y, int x) {
	/* TODO: think about coordinate conversions earlier on in the program */
	if ( y >= 6 && y <= 22 ) {
		y-=6; /* convert y so y origin is 6 */
		
		int temp = y*12;
		
		if ( x >= 8 && x <= 19 ) {
			/* left half of board */
			x -= 8; /* convert x so x origin is 8 */
			
			return temp + (x % 12);
			
		} else if (x >= 28 && x <= 39) {
			x -= 27; /* convert x so x origin is 8 TODO: fix this comment */
			
			/* right half of board */
			temp+=203;
			
			return temp + (x % 13); /* TODO: why 13? */
		} else {
			return -1; /* x is invalid */
		}
	} else {
		return -1; /* y is invalid */
	}
}

Point arraytopoint(int a) {
	
	Point result;
	
	if ( a <= 203 ) 						/* left half */
		result.x = ( a % 12 ) + START_X;
	else									/* right half */
		result.x = ( a % 12 ) + START_X + 20;
	
	int reducer = 0;
	if (a > 203)
		reducer = 17; /* TODO: revisit this name for clarity */
	result.y = START_Y + ( ( a/12 ) ) - reducer;
	
	return result;
}

char genTrash() {
	int min = 1;	/* 31 possible chars i want to pick from randomly for trash. */
	int max = 31;
	
	char c;
	int i = ( rand() % (max+1-min) ) + min; /* 1-31 */
	
	if (i <= 15) { 				/* number is between 1, 15 */
		c = i + 32;					/* ascii values between 33, 47 */
	} else if (i <= 22 ) {		/* number is between 16, 22 */
		c = i + 42;					/* ascii values between 58, 64 */
	} else if ( i <= 28 ) {		/* number is between 23, 28 */
		c = i + 68;					/* ascii values between 91, 96 */
	} else {					/* number is between 29, 31 */
		c = i + 94;					/* ascii values between 123, 125 */
	}
	
	return c;
}

void genPasswords() {			/* Fill the passwords array with Passwords */
	/* TODO: use passwordLength */
	/* TODO: randomly pick words from list */
	
	passwords[0] = "DELICACY";
	passwords[1] = "ABANDONS";
	passwords[2] = "CASHBACK";
	passwords[3] = "GREENERY";
	passwords[4] = "TADPOLES";
	passwords[5] = "KNITPICK";								
	passwords[6] = "GRUELING";
	passwords[6] = "ASSESSOR";
	passwords[7] = "CAUTIONS";
	passwords[8] = "CANNIBIS";
	passwords[9] = "ACCEPTED";
	
	int max = NUM_PASSWORDS-1;
	int min = 0;
	correct = ( rand() % (max+1-min) ) + min; /* 0, NUM_PASSWORDS-1 */
	
	/*
		TODO: implement this
		Get total number of spaces in board
		 subtract WORD_LENGTH * NUM_PASSWORDS
		Divide by NUM_PASSWORDS = var
		Randomly place word between i * (from 0 to var-1-WORD_LENGTH)
	*/
	
	/* At each passLocation copy a password */
	int currLocation = 0;
	int i;
	for ( i = 0; i < NUM_PASSWORDS; i++ ) {
		currLocation = passLocations[i];
		
		char *s = passwords[i];
		char c;
		int j = 0;
		while ( (c = s[j]) ) {
			board[currLocation] = c;
			
			j++;
			currLocation++;
		}
	}
	
	/* TODO: Randomly place hacks here and there */
}

int insideWord() { /* if inside word, return array start position, else -1 */
	int a = yxtoarray(cur.y, cur.x);
	
	int i;
	for (i = 0; i < NUM_PASSWORDS; i++) { /* iterate through all words */
		if (a >= passLocations[i] && a < (passLocations[i] + passwordLength )) {
			return passLocations[i];
		}
	}
	
	return -1;
}

void highlight() {
	attron(A_STANDOUT);
	
	int a = yxtoarray(cur.y, cur.x);
	
	if (insideWord() >= 0) {
		a = insideWord();
		while ( isalpha(board[a])  ) {
			mvprintw( arraytopoint(a).y, arraytopoint(a).x, "%c", board[a]);
			a++;
		}
		attroff(A_STANDOUT);
		return;
	}
	
	printw("%c", board[a]);
	attroff(A_STANDOUT);
}

char * stringatcursor() { 		/* TODO: refactor heavily */
	if (insideWord() >= 0) { 	/* Cursor is in word. */
		return passwords[0];	/* TODO this will only print the first password */
	} else {					/* Cursor is on a single char. */
		char *returner = malloc(sizeof(char) * 2);
		returner[0] = board[yxtoarray(cur.y, cur.x)];
		returner[1] = '\0';
		return returner;
	}
}

void mvtermprint(int y, int x, char *string, int speed) {
		
	/*	
		TODO: Debug - why does strlen return 14?
		if (len == 14) {
			mvprintw(0,0, "FAILURE");
			getch();
		}
		mvprintw(0,0, "%i ", len);
	*/
	
	int i;
	int len = strlen(string);
	
	if (speed == TYPE_SPEED && speed > 0) {
		mvprintw( y, x++, ">" );
		refresh();
	
		usleep(1000000); /* TODO: get exact time */
	}
	
	if (len == 1) {
		mvprintw( y, x++, "%c", string[0]);
		refresh();
	} else {
		for (i=0; i < len; i++) {
			mvprintw( y, x++, "%c", string[i]);
			refresh();
			usleep(speed);
		}
	}
	
	if (speed == TYPE_SPEED && speed > 0) {
		usleep(1000000);
	}
}

int numberofcorrectchars(const char *checkword) {
	if (strlen(checkword) != passwordLength) {
		/* TODO: properly free memory and end the program */
		endwin();
		fprintf(stderr, "ERROR: numberofcorrectchars called on string of length%i when passwords are of length %i.\n", (int)strlen(checkword), passwordLength);
		exit(1);
	}
	
	int count = 0;
	
	int i;
	for (i=0; i < passwordLength; i++) {
		if (passwords[correct][i] == checkword[i]) {
			count++;
		}
	}

	return count;
}

void accesssystem() {
	
	clear();
	
	mvtermprint( 1, 1, "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK", PRINT_SPEED );
	mvtermprint( 3, 1, "LOGIN ADMIN", TYPE_SPEED );	
	mvtermprint( 5, 1, "ENTER PASSWORD NOW", PRINT_SPEED );	
	
	mvprintw( 7, 1, ">");
	int i;
	for (i=0; i < passwordLength; i++) {
		mvprintw(7, 3+i, "*");
		/* TODO: Make this use mvtermprint OR implement usleep()*/
	}
	
	clear();

	mvtermprint( 1, 7, "ROBCO INDUSTRIES UNIFIED OPERATING SYSTEM", PRINT_SPEED );
	mvtermprint( 2, 9, "COPYRIGHT 2075-2077 ROBCO INDUSTRIES", PRINT_SPEED );
	mvtermprint( 3, 22, "-Server 1-", PRINT_SPEED );
	
	/* If Lock ///////////////////////////////////////////////////////////////*/
	mvtermprint( 5, 5, "SoftLock Solutions, Inc", PRINT_SPEED );
	mvtermprint( 6, 1, "\"Your Security is Our Security\"", PRINT_SPEED );
	mvtermprint( 7, 1, ">\\ Welcome, USER", PRINT_SPEED );
	mvtermprint( 8, 1, "_______________________________", PRINT_SPEED );
	
	int selection = 0;
	
	int num_options = 2;
	char *menu[] = {
		"Disengage Lock",
		"Exit"
	};
	
	/* Menu */
	while (1) {
		/* Print Menu */
		int i;
		for (i=0; i < num_options; i++) {
			if (selection == i) {		
				attron(A_STANDOUT);
				mvprintw( 9+i, 1, "                               "); /* TODO: too long? */
				mvprintw( 9+i, 1, "> %s", menu[i]);
				attroff(A_STANDOUT);
			} else {
				mvprintw( 9+i, 1, "                               "); /* TODO: too long? */
				mvprintw(9+i, 1, "> %s", menu[i]);
			}
		}
		
		mvprintw( 22, 1, "                           "); /* TODO: too long? */
		mvprintw( 22, 1, "> ");
		
		int uInput = getch();	/* Get user input from keyboard (pause) */
		usleep(10); 			/* Reduces cursor jump if arrows are held down */ 
		
		switch (uInput) {
		case '\n' :				/* TODO: test Linux */
			mvtermprint(22, 3, menu[selection], PRINT_SPEED );
			if (getch() == '\n') {
				mvprintw( 22, 1, "                           "); /* TODO: too long? */
				mvprintw( 22, 1, "> ");
				mvtermprint( 22, 3, "Exiting...", PRINT_SPEED);
				usleep(1000000*3);
				exituos();
			}
			break;
		case KEY_UP :
			selection == 0 ? selection = 1 : selection--;
			break;
		case KEY_DOWN :
			selection == 1 ? selection = 0 : selection++;
			break;
		case 'q' : /* Quit key */
			exituos();
		default:
			break;
		}
	}
	
	/* TODO: print ">" with blinking cursor at bottom */

	exituos();
}

void exituos() {
	/* free all alloc'ed memory */
	free(hacks);
	free(messages);
	free(passwords);
	/* free(board); */
	/* free(registers); */
	
	endwin(); /* End ncurses mode */
	exit(0);  /* Exit with success error code */
}

void lockterminal() {
	/* TODO: Find a way to move the entire screen up, line by line until no
	         lines are visible. */
	
	curs_set(0); /* Hide the cursor */
	
	erase();
	
	int len = strlen("TERMINAL LOCKED");
	int centerX = cols/2;
	int centerY = rows/2;
	
	mvprintw(centerY-5, centerX-(len/2), "TERMINAL LOCKED");
	len = strlen("PLEASE CONTACT AN ADMINISTRATOR");
	mvprintw(centerY-3, centerX-(len/2), "PLEASE CONTACT AN ADMINISTRATOR");
	
	getch();
	exituos();
}
