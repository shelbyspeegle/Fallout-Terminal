#include "utilities.h"

char genTrash() {
	int min = 1;	/* 31 possible chars i want to pick from randomly for trash. */
	int max = 31;
	
	char c;
	int i = ( rand() % (max+1-min) ) + min; /* 1-31 */
	
	if (i <= 15) { 				/* number is between 1, 15 */
		c = (char) (i + 32);					/* ascii values between 33, 47 */
	} else if (i <= 22 ) {		/* number is between 16, 22 */
		c = (char) (i + 42);					/* ascii values between 58, 64 */
	} else if ( i <= 28 ) {		/* number is between 23, 28 */
		c = (char) (i + 68);					/* ascii values between 91, 96 */
	} else {					/* number is between 29, 31 */
		c = (char) (i + 94);					/* ascii values between 123, 125 */
	}
	
	return c;
}

int yxtoarray(int y, int x) {
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