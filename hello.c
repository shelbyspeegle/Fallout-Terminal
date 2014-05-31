
#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <string.h> 

char *registers[32];

void printregisters();

int main() {
	char top[]="ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL\nENTER PASSWORD NOW\n";

	int row, col;

	initscr();						/* start the curses mode */
	getmaxyx(stdscr,row,col);		/* get the number of rows and columns */

	// Window is not big enough
	if (row < 24 || col < 56) 
		return 0; //TODO: better error handling

	// board set-up ///////////////////////////////////////////////////////////
	mvprintw(1, 0, "%s", top);

	registers[ 0] = "0xF964 ,";
	registers[ 1] = "0xF970 ,";
	registers[ 2] = "0xF97C ,";
	registers[ 3] = "0xF988 ,";
	registers[ 4] = "0xF994 ,";
	registers[ 5] = "0xF9A0 ,";
	registers[ 6] = "0xF9AC ,";
	registers[ 7] = "0xF9B8 ,";
	registers[ 8] = "0xF9C4 ,";
	registers[ 9] = "0xF9D0 ,";
	registers[10] = "0xF9DC ,";
	registers[11] = "0xF9E8 ,";
	registers[12] = "0xF9F4 ,";
	registers[13] = "0xFA00 ,";
	registers[14] = "0xFA0C ,";
	registers[15] = "0xFA18 ,";
	registers[16] = "0xFA24 ,";

	printregisters();
	
	move(6, 7);

	int curY;
	int curX;
	getyx(stdscr, curY, curX);

	int trysLeft = 4;
	while (1) {	
		switch (trysLeft) {
			case 4: 
				mvprintw(4, 21, "# # # #");
				break;
			case 3:
				mvprintw(4, 21, "# # #  ");
				break;
			case 2:
				mvprintw(4, 21, "# #    ");
				break;
			case 1: // Lockout imminent
				attron(A_BLINK);
				mvprintw(2, 0, "!!! WARNING: LOCKOUT IMMINENT !!!");
				attroff(A_BLINK);
				refresh();
				mvprintw(4, 21, "#      ");
				break;
			default:
				// LOSE  █
				break;
		}	
			
		mvprintw(4, 0, "%i ATTEMPT(S) LEFT : ", trysLeft);
		refresh();
		move(curY, curX);
		
		printw(0, 0, "%i %i", curY, curX);
		
		char uInput = getch();
		
		if (uInput  == 'd')
			move(curY, ++curX);
		refresh();
		getch();
	}

	endwin();

	return 0;
}

void printregisters() {
	int numregisters = 17;
	int i;
	for (i = 0; i < numregisters; i++) {	
		mvprintw(6+i, 0, "%s", registers[i]);
	}
}
