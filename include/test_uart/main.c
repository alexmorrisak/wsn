#include <msp430.h>
#include "myuart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



void main(void){
  int i, len;
  int helpFlag = 1;
  char buffer[256];

  IncrementVcore();
  IncrementVcore();
  IncrementVcore();

  initClock();
  initUART();
  sleep(1);
  _EINT();

  while (1) {
  // UART Loopback test
    if (helpFlag) {
        uartPrintf("\n\n\nType whatever you want. The MCU will repeat it back to you 50x.\n");
        helpFlag = 0;
    }
    uartPrintf("\n>> ");
    len = uartReceive(buffer); // This will block until we receive a newline
    //toggleLED();
    buffer[len] = '\0';
    for (i=0;i<50;i++) {
        uartPrintf("%s", buffer);
    }
    //uartTransmit(buffer, len);
  }// end while loop
}

