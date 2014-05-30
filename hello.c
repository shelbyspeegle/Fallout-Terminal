
#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <string.h> 
 
int main() {

	char top[]="ROBCO INDUSTRIES (TM) █ TERMLINK PROTOCOL\nENTER PASSWORD NOW\n";

	int row,col;

	initscr();						/* start the curses mode */
	getmaxyx(stdscr,row,col);		/* get the number of rows and columns */

	// board set-up ///////////////////////////////////////////////////////////////////
	
	mvprintw(1, 0, "%s", top);
	
	char *registers[32];

	registers[0] = "0xF92C ,"; 

	int trysLeft = 4;
	while (1) {	
		switch (trysLeft) {
			case 4: 
				mvprintw(5, 21, "# # # #");
				break;
			case 3:
				mvprintw(5, 21, "# # #  ");
				break;
			case 2:
				mvprintw(5, 21, "# #    ");
				break;
			case 1:
				mvprintw(5, 21, "#      ");
				break;
			default:
				// LOSE  █
				break;
		}	
			
		mvprintw(5, 0, "%i ATTEMPT(S) LEFT : ", trysLeft);
		refresh();	
			
		mvprintw(7, 0, "%s", registers[0]);

		move(7, 7);
		if (getch() == 'q')
			break;
		else
			trysLeft--;
	}

	endwin();

	return 0;
}
