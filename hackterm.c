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

#define MAX_MESSAGES 15
#define MAX_MESSAGE_LENGTH 13

// Difficulty	Length
// Very Eary	4-5
// Easy			6-8
// Average		9-10
// Hard			11-12
// Very Hard	13-15

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

			// Repeat of the first 17 //TODO: replace with real Registers
			"0xF964", "0xF970", "0xF97C", "0xF988", "0xF994", "0xF9A0", "0xF9AC", 
			"0xF9B8", "0xF9C4", "0xF9D0", "0xF9DC", "0xF9E8", "0xF9F4", "0xFA00", 
			"0xFA0C", "0xFA18", "0xFA24"
		};

char **messages;
char board[408]; // board of 408 chars ()

void printregisters();
void printinputarea();
void printboard();
void pushMessage(const char *newMessage);
int tryPassword();
void removeDud();
void addPasswordsToBoard();
int curXYToArray(int x, int y);
int getYfromArray(int a);
int getXfromArray(int a);
char genTrash();

int main() {
	messages = malloc( sizeof(char*) * MAX_MESSAGES);
	
	int rows, cols;
	
	// add trash to board
	int i;
	for (i = 0; i < 408; i++) {
		board[i] = genTrash();
	}
	

	initscr();						// start the curses mode
	noecho();						// silence user input
	getmaxyx(stdscr,rows,cols);		// get the number of rows and columns

	if (rows < 24 || cols < 56)		// Check to see if window is big enough
		return 0; 					//TODO: better error handling

	keypad(stdscr, TRUE);
	// Board Set-up ///////////////////////////////////////////////////////////
	mvprintw(1, 1, "ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL\n ENTER PASSWORD NOW\n");

	printregisters();

	move(6, 8);

	int curY;
	int curX;

	getyx(stdscr, curY, curX);
	
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
				attron(A_BLINK);
				mvprintw(2, 1, "!!! WARNING: LOCKOUT IMMINENT !!!");
				attroff(A_BLINK);
				refresh();
				mvprintw(4, 22, "#      ");
				break;
			default:
				// LOSE
				//TODO: exit and notify user
				break;
		}

		mvprintw(4, 1, "%i ATTEMPT(S) LEFT : ", trysLeft);
		printinputarea();
		printboard();
		refresh();
		move(curY, curX);
		

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
		case 'q' : // quit
			endwin();
			return 0;
		default:
			break;
		}
		//refresh();
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
	mvprintw(22, 41, ">DANGERS"); // example

	int i;
	for (i=0; i < MAX_MESSAGES; i++) {
		if (messages[i])
			mvprintw(20-i, 41, "%s", messages[i]); // Only print messages that exist
	}
}

void printboard() {
	int i;
	for (i = 0; i < 408; i++) { // iterate through each char in array
		char a = board[i];
		// if ( a == 'n' || a == '1' || a == '2' )
			mvprintw(getYfromArray(i), getXfromArray(i), "%c", a);
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
	
}

int curXYToArray(int x, int y) {
	
	// if x < 6 || x > 22
	// if y < 8 || y > 39
	// TODO: gutter check
	
	//  0  1  2  3  4  5  6  7  8  9 10 11
	// 12 13 14 15 16 17 18 19 20 21 22 23
	//                               .. 203
	
	// 6, 8  = 0
	// 6, 9  = 1
	// 6, 10 = 2
	// 6, 11 = 3
	// 6, 12 = 4
	// 6, 13 = 5
	// 6, 14 = 6
	// 6, 14 = 7
	// 6, 15 = 8
	// 6, 16 = 9
	// 6, 17 = 10
	// 6, 18 = 11
	
	// 7, 8  = 12
	
	// starts at x = 8, ends 19, 28 - 39
	// starts at y = 6, ends at y = 22
	
	// < 204 is left
		// 13 per line
	
	// > 203 is right
	
	return -1;
}

int getYfromArray(int a) { //TODO: combine these two functions using a "Point"
	int startY = 6;
	int reducer = 0;
	if (a > 203)
		reducer = 17; //TODO: revisit this name for clarity
	return startY + ( ( a/12 ) ) - reducer;
}

int getXfromArray(int a) {
	int startX = 8;
	
	if ( a <= 203 ) 					// left half
		return ( a % 12 ) + startX;
	else								// right half
		return ( a % 12 ) + startX + 20;
}

char genTrash() {
	
	int min = 1;	// 31 possible chars i want to pick from randomly
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
	
	
	/*
	! = 33
	" = 34
	# = 35
	$ = 36
	% = 37
	& = 38
	' = 39
	( = 40
	) = 41
	* = 42
	+ = 43
	, = 44
	- = 45
	. = 46
	/ = 47
          no 48 - 57
	: = 58
	; = 59
	< = 60
	= = 61
	> = 62
	? = 63
	@ = 64
		  no 65 - 90
	[ = 91
	\ = 92
	] = 93
	^ = 94
	_ = 95
	` = 96
           no 97 - 122
	{ = 123
	| = 124
	} = 125
	*/
	
	return c;
}




