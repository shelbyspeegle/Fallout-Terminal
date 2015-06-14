/*
 * hackterm.c
 *
 *  Created on: May 3, 2014
 *      Author: shelbyspeegle
 */

#include <stdlib.h>
#include <time.h>  /* for srand(time(NULL)) */
#include "game_controller.h"

int main( int argc, char **argv ) {
  srand( (unsigned int)time(0) );  /* Seed rand with this so it is more random. */

  int debugStatus = (argc == 2 && argv[1][0] == 'd');
  setDebugStatus( debugStatus );

  run();

  return EXIT_SUCCESS;
}