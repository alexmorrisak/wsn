#include <msp430.h>
#include "myuart.h"
#include "myspi.h"
#include "miscellaneous.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



void main(void){
  int i, j, len;
  int helpFlag = 1;
  char buffer[256];
  
  IncrementVcore();
  IncrementVcore();
  IncrementVcore();

  initClock();
  initSPI();
  initSPI(); // need to init this a 2nd time, otherwise SPI fails to transmit the first char. WHY?
  initUART();
  sleep(1);
  _EINT();

  while (1) {
    // SPI Loopback test
    if (helpFlag) {
        uartPrintf("\n\n\nType whatever you want.\nThe MCU will run it through the SPI bus and repeat it back to you.\nDon't forget to connect the SPI MOSI to MISO.\n");
        helpFlag = 0;
    }
    uartPrintf("\n>> ");

    len = uartReceive(buffer); // This will block until we receive a newline
    toggleLED();
    spiTransmit(buffer, len);
    len = spiReceive(buffer);
    uartTransmit(buffer, len);
  }// end while loop
}

