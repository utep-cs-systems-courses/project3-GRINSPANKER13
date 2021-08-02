#include "lcdutils.h"

/*
  Winning is based on color, if you sucessfully move the shape to
  the side with the matching color, you win. A simple way to check,
  is by checking the shape color, and then if the row or column is
  touching that matching color.
*/
char calculateWin(unsigned char col, unsigned char row, unsigned int color) {
  char retVal = 0;
  if ((color == COLOR_BLUE) && (row <= 10))
    retVal = 1;
  if ((color == COLOR_GREEN) && (col <= 10))
    retVal = 1;
  if ((color == COLOR_RED) && (row + 20 >= screenHeight - 10))
    retVal = 1;
  if ((color == COLOR_YELLOW) && (col + 20 >= screenWidth - 10))
    retVal = 1;
  return retVal;
}
