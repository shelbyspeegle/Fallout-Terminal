#include "utilities.h"

char genTrash() {
  char c;
  int i = ( rand() % 31 ) + 1;  /* Random number between 1-31 */

  if (i <= 15) {           /* Number is between 1, 15-        */
    c = (char) (i + 32);   /*   ascii values between 33, 47   */
  } else if (i <= 22 ) {   /* Number is between 16, 22-       */
    c = (char) (i + 42);   /*   ascii values between 58, 64   */
  } else if ( i <= 28 ) {  /* Number is between 23, 28-       */
    c = (char) (i + 68);   /*   ascii values between 91, 96   */
  } else {                 /* Number is between 29, 31-       */
    c = (char) (i + 94);   /*   ascii values between 123, 125 */
  }

  return c;
}

int arrayFromYX(int y, int x) {
  if ( y >= 6 && y <= 22 ) {
    y -= START_Y;  /* Convert y so y origin is 6. */

    int temp = y*12;

    if ( x >= 8 && x <= 19 ) {
      /* Left half of board. */
      x -= START_X;  /* Convert x so x origin is 8. */

      return temp + (x % 12);

    } else if (x >= 28 && x <= 39) {
      x -= 27;  /* Convert x so x origin is 8. */

      /* Right half of board. */
      temp+=203;

      return temp + (x % 13); /* TODO: Research why 13 and not 12. */
    } else {
      return -1;  /* X is invalid. */
    }
  } else {
    return -1;  /* Y is invalid. */
  }
}

Point arrayToPoint(int a) {
  Point result;

  result.x = START_X + ( a%12 );
  result.y = START_Y + ( a/12 );

  if ( a > 203 ) {
    result.x += 20;  /* Add 20 if in the right half. */
    result.y -= 17;
  }

  return result;
}