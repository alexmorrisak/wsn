#include <msp430.h>
#include "myuart.h"
#include "myspi.h"
#include "miscellaneous.h"
#include "sx1262.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



void main(void){
  int i, j, len;
  int helpFlag = 1;
  char buffer[256];
  char data[256];
  //char uartBuffer[256];
  struct Status status;
  struct RxBufferStatus rxBufferStatus;
  char standbyMode = 0;
  char packetType = 0;
  int syncword = 0;
  int irqStatus;
  unsigned long freq;
  char rssi;
  
  IncrementVcore();
  IncrementVcore();
  IncrementVcore();

  initClock();
  initSPI();
  initSPI(); // need to init this a 2nd time, otherwise SPI fails to transmit the first char. WHY?
  initUART();
  initInterruptPin();
  sleep(1);
  _EINT();

  while (1) {
 // sx1262 Transmit Test
    uartPrintf("Enter RF frequency setting\n>> ");
    len = uartReceive(buffer); // This will block until we receive a newline
    buffer[len] = '\0';
    freq = atol(buffer);
    uartPrintf("You entered: %lu\n", freq);

    status = getStatus();
    uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);

    sleep(1);
    setDIO2AsRfSwitchCtrl(1);

    sleep(1);
    uartPrintf("\n\n\nSetting to standby mode...\n");
    setStandby(STANDBY_RC);

    //sleep(100);
    uartPrintf("Setting PA Config...\n");
    setPaConfig(0x04, 0x03, 0x00);

    sleep(1);
    uartPrintf("Setting RF frequency to %lu MHz...\n", freq);
    setRfFrequency(freq);

    sleep(1);
    uartPrintf("Setting Tx params...\n");
    setTxParams(0x06, 0x07);

    uartPrintf("Setting modulation parameters...\n");
    setModulationParams();

    uartPrintf("Setting the packet params...\n");
    setPacketParams();

    //sleep(100);
    uartPrintf("Going to Tx mode...\n");

    setTxContinuousWave();

    sleep(1);
    status = getStatus();
    uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);

    }// end while loop
}

