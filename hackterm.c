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

#define MAX_MESSAGES 15
#define MAX_MESSAGE_LENGTH 13
#define NUM_PASSWORDS 10 // i think it is 17
#define NUM_HACKS 5 //TODO: find the real number

// Difficulty	Length
// Very Eary	4-5
// Easy			6-8
// Average		9-10
// Hard			11-12
// Very Hard	13-15

int passwordLength = 8; //TODO: make dynamic

int passLocations[] = {0,12,24,36,48,60,72,84,96,108}; //TODO: make dynamic

char *registers[] = {
			"0xF964",
			"0xF970",
			"0xF97C",
			"0xF988",
			"0xF994",
			"0xF9A0",
			"0xF9AC",
			"0xF9B8",
			"0xF9C4",
			"0xF9D0",
			"0xF9DC",
			"0xF9E8",
			"0xF9F4",
			"0xFA00",
			"0xFA0C",
			"0xFA18",
			"0xFA24",

			// Repeat of the first 17 //TODO: replace with real registers
			"0xF964", "0xF970", "0xF97C", "0xF988", "0xF994", "0xF9A0", "0xF9AC", 
			"0xF9B8", "0xF9C4", "0xF9D0", "0xF9DC", "0xF9E8", "0xF9F4", "0xFA00", 
			"0xFA0C", "0xFA18", "0xFA24"
		};

char **messages;							// Array of message strings.
char board[408]; 							// Board of 408 chars.
int hackLocations[NUM_HACKS];				// Series of array positions where the
//int passLocations[NUM_PASSWORDS];			//   hacks and passwords lie on the board.
char **hacks;
char **passwords;
int curY, curX;


void printregisters();
void printinputarea();
void printboard();
void pushMessage(const char *newMessage);
int tryPassword();
void removeDud();
void addPasswordsToBoard();
int yxtoarray(int y, int x);
int arraytoy(int a);
int arraytox(int a);
char genTrash();
void genPasswords();
int insideWord();
void highlight();

int main(int argc, char **argv) {
	//TODO: create ending function, includes endwin() and return 0
	
	int rows, cols;
	
	initscr();						// start the curses mode
	noecho();						// silence user input
	getmaxyx(stdscr,rows,cols);		// get the number of rows and columns
	keypad(stdscr, TRUE);			// converts arrow key input to usable chars
	curs_set(0);					// This hides the ncurses cursor

	if (rows < 24 || cols < 56) {	// Check to see if window is big enough
		endwin();
		printf("ERROR: Terminal window is too small,\n");
		printf("       minimum size of 24x56 to run.\n");
		return 0;
	}

	
	// Board Set-up ///////////////////////////////////////////////////////////
	messages = malloc( sizeof(char*) * MAX_MESSAGES); //TODO: free
	hacks = malloc( sizeof(char*) * NUM_PASSWORDS);   //TODO: free
	passwords = malloc( sizeof(char*) * NUM_HACKS);   //TODO: free
	mvprintw(1, 1, "ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL\n ENTER PASSWORD NOW\n");

	// add trash to board
	int i;
	for (i = 0; i < 408; i++) {
		board[i] = genTrash();
	}
	
	genPasswords();
	addPasswordsToBoard();
	
	printregisters();

	move(6, 8);
	
	getyx(stdscr, curY, curX);		// Get the cursor position and assign to curY, curX
	int trysLeft = 4;
	
	while (1) {
		switch (trysLeft) {
			case 4:
				mvprintw(4, 22, "# # # #");
				break;
			case 3:
				mvprintw(4, 22, "# # #  ");
				break;
			case 2:
				mvprintw(4, 22, "# #    ");
				break;
			case 1: // Lockout imminent
				attron(A_BLINK); // eh blinkin'!
				mvprintw(2, 1, "!!! WARNING: LOCKOUT IMMINENT !!!");
				attroff(A_BLINK);
				refresh();
				mvprintw(4, 22, "#      ");
				break;
			default:
				// LOSE
				//TODO: notify user
				erase();
				mvprintw(1,1, "LOCKED OUT, PLEASE CONTACT AN ADMINISTRATOR.");
				getch();
				endwin();
				return 0;
		}

		mvprintw(4, 1, "%i ATTEMPT(S) LEFT : ", trysLeft);
		mvprintw(22, 41, ">");
		
		printinputarea();
		printboard();
		
		move(curY, curX);
		
		highlight();
		
		refresh();
		
		// getting user input from keyboard
		int uInput = getch();
		usleep(10); // Reduces cursor jump when arrows are held down
		
		switch (uInput) {
		case KEY_UP :
			if (curY != 6)
				curY--;
			break;
		case KEY_LEFT :
			if (curX == 28)
				curX = 19;
			else if (curX != 8)
				curX--;
			break;
		case KEY_DOWN :
			if (curY != 22)
				curY++;
			break;
		case KEY_RIGHT :
			if (curX == 19)
				curX = 28;
			else if (curX != 39)
				curX++;
			break;
		case 'e' :
			removeDud();
			break;
		case '-' :
			trysLeft--;
			break;
		case 'q' : // Quit key
			endwin();
			return 0;
		default:
			break;
		}
		
		// if inside a word
			// if key right
				// if not at MAX, go to arraystartposition + WORDLENGTH + 1
			// if key left
				// if not at 0, go to arraystartposition of word - 1
	}

	endwin();
	return 0;
}

void printregisters() {
	int numregisters = 17;
	int i;
	for (i = 0; i < numregisters; i++) {
		mvprintw(6+i, 1, "%s", registers[i]);
		mvprintw(6+i, 21, "%s", registers[i+17]);
	}
}

void printinputarea() {
	mvprintw(22, 42, "DANGERS"); // example

	int i;
	for (i=0; i < MAX_MESSAGES; i++) {
		if (messages[i])
			mvprintw(20-i, 41, "%s", messages[i]); // Only print messages that exist
	}
}

void printboard() {
	int i;
	for (i = 0; i < 408; i++) { // iterate through each char in array
		mvprintw(arraytoy(i), arraytox(i), "%c", board[i]);
	}
}

void pushMessage(const char *newMessage) {
	char *fullMsg = "             ";
	fullMsg = (char*) newMessage;
	
	int i;
	for (i=0; i < MAX_MESSAGES; i++) {
		messages[MAX_MESSAGES-i] = messages[MAX_MESSAGES-i-1];
	}
	messages[0] = fullMsg;
}

int tryPassword() { // Unimplemented
	// If hack
		//pushMessage(hackContents);
		//pushMessage("Dud removed.");
		//prints 2 lines
	// If wrong password
		// prints 3 lines
			// WORD (in all caps)
			// Entry denied
			// 0/5 correct. (varies)
	// If trash
		return -1;
}

void removeDud() {
	pushMessage(">Dud removed.");
	pushMessage(">[*(>]       ");
}

void addPasswordsToBoard() {
	
	int currLocation;
		
	int i;
	for ( i = 0; i < NUM_PASSWORDS; i++ ) {
		currLocation = passLocations[i];
		// starting at hackLocation, copy each password to the board, char by char
		char *s = passwords[i];
		char c;
		
		int j = 0;
		while ( (c = s[j]) ) {
			board[currLocation] = c;
			
			j++;
			currLocation++;
		}
	}
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

char genTrash() { //TODO: investigate deterministic behavior, trash is always the same
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

void genPasswords() {					// Fill the passwords array with Passwords
	//TODO: use passwordLength
	int i;
	for (i=0; i < NUM_PASSWORDS; i++) {
		passwords[i] = "ABCDEFGH";
	}
}

int insideWord() { // if inside word, return array start position, else -1
	
	int a = yxtoarray(curY, curX);
	
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
	
	int a = yxtoarray(curY, curX);
	
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

