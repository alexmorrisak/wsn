#include <msp430.h>
#include "myuart.h"
#include "miscellaneous.h"
#include "mytimer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void main(void) {
  unsigned int fine;
  int len;
  unsigned long coarse, waitTime;
  char buffer[256];

  IncrementVcore();
  IncrementVcore();
  IncrementVcore();

  initClock();
  initUART();
  initTimer();
  sleep(1);
  _EINT();

  while(1) {
    uartPrintf("How long do you want to wait?:\n>>");
    len = uartReceive(buffer);
    buffer[len] = '\0';
    waitTime = atol(buffer);
    uartPrintf("Wait time: %lu\n", waitTime);
    getTime(&coarse, &fine);
    uartPrintf("Current time: %lu, %u\n", coarse, fine);
    //uartPrintf("Going to wait until: %lu\n", coarse+waitTime);
    sleepUntil(coarse+waitTime);
    getTime(&coarse, &fine);
    uartPrintf("Done sleeping\n");
    uartPrintf("Current time: %lu, %u\n", coarse, fine);
  }
}
