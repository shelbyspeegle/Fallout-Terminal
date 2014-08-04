#ifndef __COMMON_H
#define __COMMON_H

typedef int boolean;
#define TRUE 1
#define FALSE 0
#define START_Y 6
#define START_X 8
#define EXIT_SUCCESS 0
#define EXIT_ERROR 1

struct point {
  int y;
  int x;
};
typedef struct point Point;

#endif /* __COMMON_H */