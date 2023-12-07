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
char deviceID;
  
IncrementVcore();
IncrementVcore();
IncrementVcore();

initClock();
initSPI();
initSPI(); // need to init this a 2nd time, otherwise SPI fails to transmit the first char. WHY?
initUART();
initInterruptPin();
initTemp();
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

    
      switch(deviceID){//device ID determines master slave1 or slave2
      case 1://master case
        /*
         wake up
         go to transmit mode
         send request for packet from other nodes
         go to receive mode
         slave 1 responds after xx time
         we print its deviceID, time stamp, packet number, and temperature to UART 
         slave 2 waits xx time and then sends its own
         we print its deviceID, time stamp, packet number, and temperature to UART
         everyone sleeps until 5 sec pass
         repeat
        */
      
      //start timer
      while(1){
        /*
        data = getTime();
        len = size(data);
        writeBuffer(data, 0, len);
        setTx();

        LPM0;//SLEEP UNTIL WE ARE DONE TRANSMITTING
        setRx();
        LPM0;//sleep till we receive message 1
        rxBufferStatus = getRxBufferStatus();
        readBuffer(data, rxBufferStatus.RxStartBufferPointer, rxBufferStatus.PayloadLengthRx)
        setRx();
        LPM0;//sleep till we receive message 2

        /*
        
      }   

      break;
      case 2://slave 1 case
      /*
        wake up
        go into receive mode
        receive packet request
        wait xx time
        load transmit buffer w deviceID, time stamp, packet number, and temperature
        go into transmit mode
        sleep until the start of the next frame
      */

      break;
      case 3://slave 2 case
      /*
        wake up
        go into receive mode
        receive packet request
        wait xx time
        load transmit buffer w deviceID, time stamp, packet number, and temperature
        go into transmit mode
        sleep until the start of the next frame
      */

      break;

      
  
}
   