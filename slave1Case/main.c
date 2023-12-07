#include <msp430.h>
#include "myuart.h"
#include "myspi.h"
#include "miscellaneous.h"
#include "sx1262.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ADC.c"
#include <mytimer.h>
 
 /*
        wake up
        go into receive mode
        receive packet request
        wait xx time
        load transmit buffer w deviceID, time stamp, packet number, and temperature
        go into transmit mode
        sleep until the start of the next frame
*/


char deviceID = 2;
char packetNumber = 0;
int temperature;

int main(void) {
    int i, j, len;
    char npackets=0;
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
    char deviceID = 2;
    unsigned int packetNumber = 0;

    unsigned long coarse, waitTime;
    unsigned int fine;
 
    IncrementVcore();
    IncrementVcore();
    IncrementVcore();

    initClock();
    initSPI();
    initSPI(); // need to init this a 2nd time, otherwise SPI fails to transmit the first char. WHY?
    initUART();
    initInterruptPin();
    initTemp();
    initTimer();
    sleep(1);
    _EINT();


    // Initialize the radio
    uartPrintf("\n\n\nSetting to standby mode...\n");
    setStandby(STANDBY_RC);

    status = getStatus();
    uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);

    uartPrintf("Setting to LoRA packet type...\n");
    setPacketType(PACKET_TYPE_LORA);

    uartPrintf("Setting RF frequency to 915 MHz...\n");
    setRfFrequency(915);

    // This works
    sleep(1);
    uartPrintf("Setting Tx params...\n");
    setTxParams(0xf7, 0x07);

    sleep(1);
    uartPrintf("Setting PA Config...\n");
    setPaConfig(0x02, 0x02, 0x00);

    sleep(1);
    setDIO2AsRfSwitchCtrl(1);

    uartPrintf("Setting buffer base addresses...\n");
    setBufferBaseAddress(0, 128);

    uartPrintf("Setting modulation parameters...\n");
    setModulationParams();

    uartPrintf("Setting the packet params...\n");
    setPacketParams();

    uartPrintf("Setting interrupt mask...\n");
    //setDioIrqParams(0xff, 0xff, 0, 0); // turn on all interrupts, map them to DIO1
    setDioIrqParams(IRQ_TXDONE | IRQ_RXDONE, 0xff, 0, 0); // turn on RXDONE interrupts, map all  to DIO1

    setLoraSyncWord(0x1424);

while(1){
  
  
  setRx(0); //get initial packet from master containing packet request
  getTime(&coarse, &fine);
  sleepUntil(coarse + 10);
  status = getStatus();
  uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);

  while(!radioInterruptFlag) {
  LPM0;
  }

  irqStatus = getIrqStatus();
  uartPrintf("Interrupt vector: 0x%02x\n", irqStatus);
  clearIrqStatus(irqStatus);
  radioInterruptFlag = 0;
  
  getTime(&coarse, &fine);
  sleepUntil(coarse + 10);
  
  getTemp();
  temperature = (int)(temp*100);
  packetNumber++;
  getTime(&coarse, &fine);

             // Set the data for the data payload
  buffer[0] = deviceID;
  buffer[1] = (packetNumber >> 8) & 0xff;
  buffer[2] = (packetNumber >> 0) & 0xff;
  buffer[3] = (coarse >> 24) & 0xff;
  buffer[4] = (coarse >> 16) & 0xff;
  buffer[5] = (coarse >>  8) & 0xff;
  buffer[6] = (coarse >>  0) & 0xff;
  buffer[7] = (fine >> 8) & 0xff;
  buffer[8] = (fine >> 0) & 0xff;
  buffer[9] = (temperature >> 8) & 0xff;
  buffer[10] = (temperature >> 0) & 0xff;
  buffer[11] = 0x00;
  buffer[12] = 0x00;
  buffer[13] = 0x00;
  buffer[14] = 0x00;
  buffer[15] = 0x00;
  writeBuffer(buffer, 0, 16);

  
  setTx(0); // transmit the packet
  while(!radioInterruptFlag) {
    LPM0;
  }
  radioInterruptFlag = 0;
  clearIrqStatus(0xff);

  uartPrintf("Device ID %i, Packet # %i, Coarse %i, Fine %i.. \n", deviceID, packetNumber, coarse, fine);
  uartPrintf("Temperature: %i \n", temperature);


  //sleep till 5s total pass
  //sleepUntil(coarse + 1*64); // sleep until the next 5-second interval
         
  }

  }
