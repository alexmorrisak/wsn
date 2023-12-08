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


void main(void){
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

    char deviceID = 3;
    int packetNumber = 0;
    int temperature;

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
    setTxParams(0x16, 0x07);

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

    for (i=0; i<16; i++) {
        data[i] = 0;
    }
    
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = npackets;
    buffer[4] = 0x50;
    buffer[5] = 0x49;
    buffer[6] = 0x4e;
    buffer[7] = 0x47;
    buffer[8] = 0x08;
    buffer[9] = 0x09;
    buffer[10] = 0x0a;
    buffer[11] = 0x0b;
    buffer[12] = 0x0c;
    buffer[13] = 0x0d;
    buffer[14] = 0x0e;
    buffer[15] = 0x0f;
    // Figure out to send a transmit packet
    writeBuffer(buffer, 0, 16); // put our payload data in the radio's buffer

    clearIrqStatus(0xff);
    while (1) {
        setRx(0);

        // wait for  radio interrupt packet
        while (!radioInterruptFlag) {
            LPM0;
        }
            getTime(&coarse, &fine);

            radioInterruptFlag = 0;
            irqStatus = getIrqStatus();
           // uartPrintf("Interrupt vector: 0x%02x\n", irqStatus);
            clearIrqStatus(irqStatus);
            rssi = getPacketStatus();
            rxBufferStatus = getRxBufferStatus();
            readBuffer(data, rxBufferStatus.RxStartBufferPointer, rxBufferStatus.PayloadLengthRx);
          //  uartPrintf("Received a packet. length: %i, RSSI: %.1f dBm, data: \n", rxBufferStatus.PayloadLengthRx, -1*rssi/2.);

            data[rxBufferStatus.PayloadLengthRx] = '\0';
            uartPrintf("Received from deviceID: %i \n", data[0]);
        
           
  
  sleepUntil(coarse + 20);//wait for frame to start
  
  getTemp();
  temperature = (int)(temp*100);
  packetNumber++;

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

  uartPrintf("Device ID %i, Packet # %i, Coarse %lu, Fine %lu \n", deviceID, packetNumber, coarse, fine);
  uartPrintf("Temperature: %i \n", temperature);

  
  //sleep till 5s total pass
  sleepUntil(coarse + 50); // sleep until the next 5-second interval
            

        
    }// end while loop
}

