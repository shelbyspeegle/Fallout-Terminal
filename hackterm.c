/*
 * hackterm.c
 *
 *  Created on: May 3, 2014
 *      Author: shelbyspeegle
 */

#include <ncurses.h>			/* ncurses.h includes stdio.h */
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h> // for srand(time(NULL))

#define MAX_MESSAGES 15
#define MAX_MESSAGE_LENGTH 13
#define NUM_PASSWORDS 10 // i think it is 17
#define NUM_HACKS 5 //TODO: find the real number
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

// Difficulty	Length
// Very Easy	4-5
// Easy			6-8
// Average		9-10
// Hard			11-12
// Very Hard	13-15

char *registers[] = {
	"0xF964", "0xF970", "0xF97C", "0xF988", "0xF994", "0xF9A0", "0xF9AC",
	"0xF9B8", "0xF9C4", "0xF9D0", "0xF9DC", "0xF9E8", "0xF9F4", "0xFA00",
	"0xFA0C", "0xFA18", "0xFA24",

	// Repeat of the first 17 //TODO: replace with real registers
	"0xF964", "0xF970", "0xF97C", "0xF988", "0xF994", "0xF9A0", "0xF9AC", 
	"0xF9B8", "0xF9C4", "0xF9D0", "0xF9DC", "0xF9E8", "0xF9F4", "0xFA00", 
	"0xFA0C", "0xFA18", "0xFA24"
};

char **messages; // Array of message strings.
char board[408]; // Board of 408 chars.
int hackLocations[NUM_HACKS]; // Series of array positions where hacks are
int passLocations[] = {0,21,55,79,111,175,220,270,300,390}; //TODO: make dynamic
int passwordLength = 8; //TODO: make dynamic
char **hacks;
char **passwords;
int rows, cols;
int trysLeft = 4;
int correct; // The index position of the right password in passLocations[]
Point cur;

void setup();
void printinputarea();
void printboard();
void pushmessage(const char *newMessage);
boolean tryPassword();
void removeDud(int a);
void addPasswordsToBoard();
int yxtoarray(int y, int x);
int arraytoy(int a);
int arraytox(int a);
char genTrash();
void genPasswords();
int insideWord();
void highlight();
char *stringatcursor();
void mvtermtype(int y, int x, char *string);
int numberofcorrectchars(const char *checkword);
void accesssystem();
void exituos();

int main(int argc, char **argv) {

	srand(time(0)); // Seed rand with this so it is more random
	
	initscr();						// start the curses mode
	noecho();						// silence user input
	getmaxyx(stdscr,rows,cols);		// get the number of rows and columns
	keypad(stdscr, TRUE);			// converts arrow key input to usable chars
	curs_set(0);					// This hides the ncurses cursor

	if (rows < 24 || cols < 55) {	// Check to see if window is big enough
		endwin();
		printf("ERROR: Terminal window is too small,\n");
		printf("       minimum size of 24x55 to run.\n");
		return 0;
		
		//53 for play area, +1 on each side for padding
	}
	
	setup();
	
	boolean loggedin = FALSE;
	
	while (1) {
		
		switch (trysLeft) {
			case 4:
			// Place the four symbols for tries left
				attron(A_STANDOUT);
				mvprintw(4, 22, " "); // 1st
				mvprintw(4, 24, " "); // 2nd
				mvprintw(4, 26, " "); // 3rd
				mvprintw(4, 28, " "); // 4th
				attroff(A_STANDOUT);
				break;
			case 3:
				mvprintw(4, 28, " "); // Erase the 4th symbol
				attron(A_STANDOUT);
				mvprintw(4, 26, " "); // 3rd
				attroff(A_STANDOUT);
				break;
			case 2:
				mvprintw(4, 26, " "); // Erase the 3rd symbol
				attron(A_STANDOUT);
				mvprintw(4, 24, " "); // 2nd
				attroff(A_STANDOUT);
				mvprintw(2, 1, "                                 ");
				break;
			case 1: // Lockout imminent
				attron(A_BLINK); // eh blinkin'!
				mvprintw(2, 1, "!!! WARNING: LOCKOUT IMMINENT !!!");
				attroff(A_BLINK);
				refresh();
				mvprintw(4, 24, " "); // Erase the 2nd symbol
				break;
			default: // Game over
				erase();
				mvprintw(1,1, "LOCKED OUT, PLEASE CONTACT AN ADMINISTRATOR.");
				getch();
				exituos();
		}

		mvprintw(4, 1, "%i ATTEMPT(S) LEFT : ", trysLeft);
		printinputarea();
		printboard();
		move(cur.y, cur.x);
		highlight();
		refresh();
		
		if (loggedin) {
			//TODO: blink cursor 4 times
			getch(); //TODO: remove this when accesssystem works
			accesssystem();
		}

		int uInput = getch();	// Get user input from keyboard (pause)
		usleep(10); 			// Reduces cursor jump if arrows are held down
		
		switch (uInput) {
		case '\n' :				//TODO: test Linux
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
			}
			break;
		case 'a' :
			getch();
			accesssystem();
			break;
		case 'q' : // Quit key
			exituos();
		default:
			break;
		}
		
		// if inside a word
			// if key right
				// if not at MAX, go to arraystartposition + WORDLENGTH + 1
			// if key left
				// if not at 0, go to arraystartposition of word - 1
	}
	exituos();
}

void setup() {
	// Board Set-up ////////////////////////////////////////////////////////////
	
	// Place cursor at starting position (board[0])
	cur.y = START_Y;
	cur.x = START_X;
	
	messages = malloc( sizeof(char*) * MAX_MESSAGES);		//TODO: free
	hacks = malloc( sizeof(char*) * NUM_HACKS);				//TODO: free
	passwords = malloc( sizeof(char*) * NUM_PASSWORDS );	//TODO: free
	mvprintw(1, 1, "ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL");
	mvprintw(2, 1, "ENTER PASSWORD NOW");

	// Populate Entire Board with Trash ////////////////////////////////////////
	int i;
	for (i = 0; i < 408; i++) {
		board[i] = genTrash();
	}
	
	genPasswords();
	
	// Print Registers /////////////////////////////////////////////////////////
	int numregisters = 17;

	for (i = 0; i < numregisters; i++) {
		mvprintw(6+i, 1, "%s", registers[i]);
		mvprintw(6+i, 21, "%s", registers[i+17]);
	}
}

void printinputarea() {
	mvprintw( 22, 41, ">" );
	
	// Clear line
	mvtermtype( 22, 41, "              ");
	mvtermtype( 22, 42, stringatcursor() );

	attron(A_STANDOUT);
	//TODO: print # after input, looking like a cursor
	attroff(A_STANDOUT);
	

	int i;
	for (i=0; i < MAX_MESSAGES; i++) {
		if (messages[i])
			mvprintw(20-i, 41, "%s", messages[i]); // Print messages that exist
	}
}

void printboard() {
	int i;
	for (i = 0; i < 408; i++) { // iterate through each char in array
		mvprintw(arraytoy(i), arraytox(i), "%c", board[i]);
	}
}

void pushmessage(const char *newMessage) {
	char *fullMsg = malloc(sizeof(char) * MAX_MESSAGE_LENGTH + 1);
	
	// Start the message off with a '>'
	fullMsg[0] = '>';
	
	// Copy all of the contents of the passed in string
	int k;
	for (k=1; k<strlen(newMessage)+1; k++) {
		fullMsg[k] = newMessage[k-1];
	}
	
	// Fill the rest of the string with spaces
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
		int k = insideWord(); // start of the word
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
			
			//free(check)

			return TRUE; //TRUE
		} else {
			pushmessage( "Entry denied" );
			char *stringbuild = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
			sprintf(stringbuild, "%i/%i correct.", i, passwordLength);
			pushmessage( stringbuild );
			trysLeft--;
		}
	}
	//TODO: test for hacks and trash
	
	// else if (insideHack() >= 0) {
	// 	pushmessage(hackContents);
	// 	pushmessage("Dud removed.");
	// 	prints 2 lines
	// 	}
	// If trash
	// 	TODO: what does trash do?

	return 0;
}

void removeDud(int a) { //TODO: unimplemented.
	pushmessage(">Dud removed.");
	pushmessage(">[*(>]");
}

int yxtoarray(int y, int x) {
	//TODO: think about coordinate conversions earlier on in the program
	if ( y >= 6 && y <= 22 ) {
		y-=6; // convert y so y origin is 6
		
		int temp = y*12;
		
		if ( x >= 8 && x <= 19 ) {
			// left half of board
			x -= 8; // convert x so x origin is 8
			
			return temp + (x % 12);
			
		} else if (x >= 28 && x <= 39) {
			x -= 27; // convert x so x origin is 8 //TODO: fix this comment
			
			// right half of board
			temp+=203;
			
			return temp + (x % 13); // TODO: why 13?
		} else {
			return -1; // x is invalid
		}
	} else {
		return -1; // y is invalid
	}
}

int arraytoy(int a) { //TODO: combine these two functions using a "Point"
	int startY = 6;
	int reducer = 0;
	if (a > 203)
		reducer = 17; //TODO: revisit this name for clarity
	return startY + ( ( a/12 ) ) - reducer;
}

int arraytox(int a) {
	int startX = 8;
	
	if ( a <= 203 ) 						// left half
		return ( a % 12 ) + startX;
	else									// right half
		return ( a % 12 ) + startX + 20;
}

char genTrash() {
	int min = 1;	// 31 possible chars i want to pick from randomly for trash.
	int max = 31;
	
	char c;
	int i = ( rand() % (max+1-min) ) + min; // 1-31
	
	if (i <= 15) { 				// number is between 1, 15
		c = i + 32;					// ascii values between 33, 47
	} else if (i <= 22 ) {		// number is between 16, 22
		c = i + 42;					// ascii values between 58, 64
	} else if ( i <= 28 ) {		// number is between 23, 28
		c = i + 68;					// ascii values between 91, 96
	} else {					// number is between 29, 31
		c = i + 94;					// ascii values between 123, 125
	}
	
	return c;
}

void genPasswords() {			// Fill the passwords array with Passwords
	//TODO: use passwordLength
	//TODO: randomly pick words from list
	
	passwords[0] = "ABCDEFGH";
	passwords[1] = "BBCDEFGH";
	passwords[2] = "CBCDEFGH";
	passwords[3] = "DBCDEFGH";
	passwords[4] = "EBCDEFGH";
	passwords[5] = "FBCDEFGH";								
	passwords[6] = "GBCDEFGH";
	passwords[6] = "HBCDEFGH";
	passwords[7] = "IBCDEFGH";
	passwords[8] = "JBCDEFGH";
	passwords[9] = "KBCDEFGH";
	
	int max = NUM_PASSWORDS-1;
	int min = 0;
	correct = ( rand() % (max+1-min) ) + min; // 0, NUM_PASSWORDS-1
	
	//TODO: implement this
	// Get total number of spaces in board
	//  subtract WORD_LENGTH * NUM_PASSWORDS
	// Divide by NUM_PASSWORDS = var
	// Randomly place word between i * (from 0 to var-1-WORD_LENGTH)
	
	// At each passLocation copy a password
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
	
	//TODO: Randomly place hacks here and there
}

int insideWord() { // if inside word, return array start position, else -1
	int a = yxtoarray(cur.y, cur.x);
	
	int i;
	for (i = 0; i < NUM_PASSWORDS; i++) { // iterate through all words
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
			mvprintw( arraytoy(a), arraytox(a), "%c", board[a]);
			a++;
		}
		attroff(A_STANDOUT);
		return;
	}
	
	printw("%c", board[a]);
	attroff(A_STANDOUT);
}

char * stringatcursor() { //TODO: refactor heavily
	if (insideWord() >= 0) { // Cursor is in word.
		return passwords[0]; //TODO this will only print the first password
	} else {	// Cursor is on a single char.
		char *returner = malloc(sizeof(char) * 2);
		returner[0] = board[yxtoarray(cur.y, cur.x)];
		returner[1] = '\0';
		return returner;
	}
}

void mvtermtype(int y, int x, char *string) { //TODO implement a thread
	int i;
	int len = strlen(string);
	
	//TODO: Debug - why does strlen return 14?
	// if (len == 14) {
	// 	mvprintw(0,0, "FAILURE");
	// 	getch();
	// }
	// mvprintw(0,0, "%i ", len);
	
	if (len == 1) {
		mvprintw( y, x++, "%c", string[0]);
		refresh();
	} else {
		for (i=0; i < len; i++) {
			mvprintw( y, x++, "%c", string[i]);
			refresh();
			// usleep(8000);
		}
	}	
}

int numberofcorrectchars(const char *checkword) {
	if (strlen(checkword) != passwordLength) {
		//TODO: properly free memory and end the program
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

void accesssystem() { //TODO: unimplemented
	clear();
	//Print "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK"
	mvprintw( 1, 1, "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK");
	
	//Print "> LOGIN ADMIN"
	mvprintw( 3, 1, "> LOGIN ADMIN");	
	
	// "ENTER PASSWORD NOW"
	mvprintw( 5, 1, "ENTER PASSWORD NOW");	
	
	// "> ********" (passwordLength)
	mvprintw( 7, 1, ">");
	int i;
	for (i=0; i < passwordLength; i++) {
		mvprintw(7, 3+i, "*");
	}

	
	getch();
	clear();

	mvprintw( 1, 7, "ROBCO INDUSTRIES UNIFIED OPERATING SYSTEM" );
	mvprintw( 2, 9, "COPYRIGHT 2075-2077 ROBCO INDUSTRIES" );
	mvprintw( 3, 22, "-Server 6-" );
	
	// If Lock /////////////////////////////////////////////////////////////////
	mvprintw( 5, 5, "SoftLock Solutions, Inc" );
	mvprintw( 6, 1, "\"Your Security is Our Security\"" );
	mvprintw( 7, 1, ">\\ Welcome, USER" );
	mvprintw( 8, 1, "_______________________________" );
	
	int selection = 0;
	char *menu[] = {
		"Disengage Lock                             ",
		"Exit                                       "
	};
	
	
	// Menu //
	while (1) {
		// Print Menu //
		int i;
		for (i=0; i < 2; i++) {
			if (selection == i) {
				attron(A_STANDOUT);
				mvprintw(9+i, 1, "> %s", menu[i]);
				attroff(A_STANDOUT);
			} else {
				mvprintw(9+i, 1, "> %s", menu[i]);				
			}
		}
		mvprintw(22, 1, ">                                  ");
		
		int uInput = getch();	// Get user input from keyboard (pause)
		usleep(10); 			// Reduces cursor jump if arrows are held down
		
		switch (uInput) {
		case '\n' :				//TODO: test Linux
			mvtermtype(22, 3, menu[selection]);
			if (getch() == '\n') {
				exituos();
			}
			break;
		case KEY_UP :
			selection == 0 ? selection = 1 : selection--;
			break;
		case KEY_DOWN :
			selection == 1 ? selection = 0 : selection++;
			break;
		case 'q' : // Quit key
			exituos();
		default:
			break;
		}
		
		
	}
	
	//TODO: print ">" with blinking cursor at bottom

	
	
	exituos();
}

void exituos() {
	//free all alloc'ed memory
		free(hacks);
		free(messages);
		free(passwords);
		// free(board);
		// free(registers);
	
	endwin(); // End ncurses mode
	exit(0);  // Exit with success error code
}