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
    // sx1262 Receive Test
      if (helpFlag) {
          
          uartPrintf("\n\n\nSetting to standby mode...\n");
          setStandby(STANDBY_RC);

          status = getStatus();
          uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);

          uartPrintf("Setting to LoRA packet type...\n");
          setPacketType(PACKET_TYPE_LORA);

          uartPrintf("Setting RF frequency to 915 MHz...\n");
          setRfFrequency(915);

          uartPrintf("Setting buffer base addresses...\n");
          setBufferBaseAddress(0, 10);

          uartPrintf("Setting modulation parameters...\n");
          setModulationParams();

          uartPrintf("Setting the packet params...\n");
          setPacketParams();

          uartPrintf("Setting interrupt mask...\n");
          //setDioIrqParams(0xff, 0xff, 0, 0); // turn on all interrupts, map them to DIO1
          setDioIrqParams(0x02, 0xff, 0, 0); // turn on RXDONE interrupts, map all to DIO1

          setLoraSyncWord(0x1424);

          helpFlag = 0;
      }
      clearIrqStatus(0xff);
      //uartPrintf("IrqStatus: 0x%x\n", getIrqStatus());
      //uartPrintf("Setting sync word...\n");
      //uartPrintf("Going to Rx mode...\n");
      setRx(0);
      //status = getStatus();
      //uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);
      //sleep(1);
      //status = getStatus();
      //uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);
      //uartPrintf("IrqStatus: 0x%x\n", getIrqStatus());
      //uartPrintf("Waiting for radio to receive packet...\n");
      waitForInterrupt();
      //uartPrintf("Num interrupts: %lu\n", numUartDefaultInterrupts);
      //uartPrintf("Press enter to get status info. Command status == 2 means we got a packet. \n>> ");
      //len = uartReceive(buffer); // This will block until we receive a newline
      status = getStatus();
      //uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);
      //uartPrintf("IrqStatus: 0x%x\n", getIrqStatus());
      if (status.command_status == 2) {
          //irqStatus = getIrqStatus();
          //uartPrintf("Irq status: 0x%x\n", irqStatus);
          rssi = getPacketStatus();
          rxBufferStatus = getRxBufferStatus();
          readBuffer(data, rxBufferStatus.RxStartBufferPointer, rxBufferStatus.PayloadLengthRx);
          //data[rxBufferStatus.PayloadLengthRx] = '\0';
          //uartPrintf("Received this many bytes: %i\n", rxBufferStatus.PayloadLengthRx);
          //uartPrintf("Received packet starts at this address: %i\n", rxBufferStatus.RxStartBufferPointer);
          uartPrintf("Received a packet of length %i, RSSI %.1f dBm: ", rxBufferStatus.PayloadLengthRx, -1*rssi/2.);
          for (i=0; i<rxBufferStatus.PayloadLengthRx; i++) {
              uartPrintf("%02x", data[i]);
          }
          uartPrintf("\n");
      }

  }// end while loop
}

