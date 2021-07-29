#include <msp430.h>
#include <libTimer.h>
#include <stdlib.h>
#include "buzzer.h"
#include "lcdutils.h"
#include "lcddraw.h"

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define LED BIT6		/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15

u_int bgColor = COLOR_WHITE;

static char switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void switch_init()			/* setup switch */
{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}


// axis zero for col, axis 1 for row
short drawPos[2] = {10,10}, controlPos[2] = {10,10};
short velocity[2] = {3,8}, limits[2] = {screenWidth-36, screenHeight-8};

short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;

void wdt_c_handler()
{
  static int secCount = 0;

  secCount ++;
  if (secCount >= 25) {		/* 10/sec */
    secCount = 0;
    redrawScreen = 1;
  }
}

void start();
void runWithInput();
void endWithWin();
char calculateWin(unsigned char row, unsigned char col, u_int color);

void main()
{
  
  P1DIR |= LED;		/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  buzzer_init();
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
  clearScreen(bgColor);
  
  while (1) {			/* forever */
    if (redrawScreen) {
      redrawScreen = 0;
      play_game();
    }
    P1OUT &= ~LED;	/* led off */
    or_sr(0x10);	/**< CPU OFF */
    P1OUT |= LED;	/* led on */
  }
}

void draw_sides()
{
  fillRectangle(0, 0, screenWidth, 10, COLOR_BLUE);
  fillRectangle(0, 10, 10, screenHeight-20, COLOR_GREEN);
  fillRectangle(0, screenHeight-10, screenWidth, 10, COLOR_RED);
  fillRectangle(screenWidth-10, 10, 10, screenHeight-20, COLOR_YELLOW);
}

void draw_shape(unsigned char col, unsigned char row, u_int color)
{
  fillRectangle(col, row, 20, 20, color);
}

char running = 0;
unsigned char currentRow, currentCol;
unsigned tempRow, tempCol;
u_int currentColors[] = { COLOR_BLUE, COLOR_GREEN, COLOR_RED, COLOR_YELLOW };
int index;
u_int shapeColor;

void play_game()
{ 
  if((switches & SW1) & (!running)) {
    running = 1;
    clearScreen(bgColor);
  }

  switch(running) {
  case 1:
    start();
    break;
  case 2: // The game is running, waiting for winner.
    runWithInput();
    break;
  case 3: // The game has been won
    endWithWin();
    break;
  default: // Before start, show instructions.
    drawString5x7(2, 4, "Simple Game:", COLOR_BLACK, bgColor);
    drawString5x7(6, 14, "move shape to side", COLOR_BLACK, bgColor);
    drawString5x7(6, 24, "with matching color", COLOR_BLACK, bgColor);
    drawString5x7(18, 84, "START [S1]", COLOR_BLACK, bgColor);
    drawString5x7(10, screenHeight-10, "Left Up Down Right", COLOR_BLACK, bgColor);
  }
}

 /*
  Start the game, draw the boundaries, randomize the shapes color,
  draw the shape center screen, and progress the state.
 */
void start()
{
    currentCol = (screenWidth / 2) - 10;
    currentRow = (screenHeight / 2) - 10;
    draw_sides();
    index = rand() % 4;
    shapeColor = currentColors[index];
    draw_shape(currentCol, currentRow, shapeColor);
    running = 2;
}

 /*
  Run the game, waiting for user input to move the shape around,
  movements are controlled by the 4 buttons provided on the board,
    Left [S1], Up [S1], Down [S3], Right [S4]
  only draw the shape if it has been moved, check for winner,
  progress state if the user has won.
 */
void runWithInput()
{
  tempRow = currentRow;
  tempCol = currentCol;
  // Buttons as directon inputs
  if (switches & SW1) currentCol--; // Left
  if (switches & SW2) currentRow--; // Up
  if (switches & SW3) currentRow++; // Down
  if (switches & SW4) currentCol++; // Right
  // Make sure you can't move the shape past the bounds
  if (currentRow <= 10) currentRow = 10;
  if (currentCol <= 10) currentCol = 10;
  if (currentRow + 20 >= screenHeight - 10) currentRow = screenHeight - 30;
  if (currentCol + 20 >= screenWidth - 10) currentCol = screenWidth - 30;
  // Draw the shape at the new location if it moved.
  if ((tempRow != currentRow) | (tempCol != currentCol)) {
    draw_shape(tempCol, tempRow, bgColor); // Erase old
    draw_shape(currentCol, currentRow, shapeColor); // Draw new
  }
  // Check to see if you are a winner.
  if (calculateWin(currentCol, currentRow, shapeColor) == 1) {
    clearScreen(bgColor);
    running = 3;
  }
}

 /*
  The user was able to move the shape to the side with the matching color!
  Ask the user if they want to play again or quit, wait for their input,
  then progress the state based on their input.
 */
void endWithWin()
{
  song2();
  drawString5x7(2, 20, "YOU WON!", COLOR_BLACK, bgColor);
  drawString5x7(2, 40, "Play again [S1]", COLOR_BLACK, bgColor);
  drawString5x7(2, 60, "Quit [S2]", COLOR_BLACK, bgColor);
  if (switches & SW1) {
    running = 1;
    clearScreen(bgColor);
    buzzer_set_period(0);
  }
  if (switches & SW2) {
    running = 0;
    clearScreen(bgColor);
    buzzer_set_period(0);
  }
}

 /*
  Winning is based on color, if you sucessfully move the shape to
  the side with the matching color, you win. A simple way to check,
  is by checking the shape color, and then if the row or column is
  touching that matching color.
 */
char calculateWin(unsigned char col, unsigned char row, u_int color) {
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

/* Switch on S2 */
void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
