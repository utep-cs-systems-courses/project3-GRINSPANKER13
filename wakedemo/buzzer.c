#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"

static int s1Counter = 0;
static int s2Counter = 0;

void buzzer_init()
{
  /* 
       Direct timer A output "TA0.1" to P2.6.  
        According to table 21 from data sheet:
          P2SEL2.6, P2SEL2.7, anmd P2SEL.7 must be zero
          P2SEL.6 must be 1
        Also: P2.6 direction must be output
  */
  timerAUpmode();                  /* used to drive speaker */
  P2SEL2 &= ~(BIT6 | BIT7);
  P2SEL &= ~BIT7;
  P2SEL |= BIT6;
  P2DIR = BIT6;                    /* enable output to speaker (P2.6) */
}

void buzzer_set_period(short cycles) /* buzzer clock = 2MHz. (period of 1k results in 2kHz tone) */
{
  CCR0 = cycles;
  CCR1 = cycles >> 1;/* one half cycle */
}

void song1(){
  switch(s1Counter){
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 6:
  case 8:
    buzzer_set_period(900);
    s1Counter++;
    break;
  case 5:
  case 7:
    buzzer_set_period(1100);
    s1Counter++;
    break;
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 15:
  case 17:
    buzzer_set_period(1000);
    if(s1Counter == 17) {
      s1Counter = 0;
    }
    else {
      s1Counter++;
    }
    break;
  case 14:
  case 16:
    buzzer_set_period(1260);
    s1Counter++;
    break;
  }
}

void song2(){
  switch(s2Counter){
  case 0:
    buzzer_set_period(500);
    s2Counter++;
    break;
  case 1:
  case 2:
  case 11:
  case 12:
    buzzer_set_period(680);
    s2Counter++;
    break;
  case 3:
  case 4:
  case 13:
  case 14:
    buzzer_set_period(540);
    if(s2Counter == 14) {
      s2Counter = 0;
    }
    else {
      s2Counter++;
    }
    break;
  case 5:
  case 7:
  case 9:
    buzzer_set_period(1050);
    s2Counter++;
    break;
  case 6:
  case 8:
  case 10:
    buzzer_set_period(500);
    s2Counter++;
    break;
  }
}
