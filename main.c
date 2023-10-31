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
    // UART Loopback test
    if (0) {
      if (helpFlag) {
          uartPrintf("\n\n\nType whatever you want. The MCU will repeat it back to you 3x.\n");
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
      //uartTransmit(buffer, len);
    }

    // SPI Loopback test
    if (0) {
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
    }

    // SX1262 basic test
    if (0) {
        if (helpFlag) {
          uartPrintf("\n\n\nPress Enter.\nThe MCU will query the status of the SX1262.\n");
          helpFlag = 0;
        }
        len = uartReceive(buffer); // This will block until we receive a newline
        buffer[0] = 0xC0;
        buffer[1] = 0x00;
        spiTransmit(buffer, 2);
        len = spiReceive(buffer);
        uartPrintf("Received: %x, %x\n", buffer[0], buffer[1]);
    }

    // sx1262 step through states test, user-interactive
    if (0) {
        if (helpFlag) {
          uartPrintf("\nSelect a state for the device\n");
          uartPrintf("2: STANDBY_RC\n");
          uartPrintf("3: STANDBY_XTAL\n");
          uartPrintf("4: FS\n");
          uartPrintf("5: RX\n");
          uartPrintf("6: TX\n");
          helpFlag = 1;
        }
        uartPrintf("\n>> ");
        len = uartReceive(buffer); // This will block until we receive a newline

        switch(buffer[0]) {
            case '2': // STANDBY_RC
                  standbyMode = 0;
                  setStandby(standbyMode);
                  uartPrintf("Setting mode to standby, mode %i...\n", standbyMode);
                  break;
            case '3': // STANDBY_XTAL
                  standbyMode = 1;
                  setStandby(standbyMode);
                  uartPrintf("Setting mode to standby, mode %i...\n", standbyMode);
                  break;
            case '4': // FS
                  setFs();
                  uartPrintf("Setting mode to frequency synthesis...\n");
                  break;
            case '5': // RX
                  setRx(0);
                  uartPrintf("Setting mode to receive...\n");
                  break;
            case '6': // TX
                  uartPrintf("Setting frequency to 915 MHz...\n");
                  setRfFrequency(915);
                  uartPrintf("Done setting frequency to 915 MHz (I hope)\n");
                  sleep(10);
                  setTx(0);
                  uartPrintf("Setting mode to transmit...\n");
                  break;
            default:
                break;
        }
        uartPrintf("\nReadback:\n");
        status = getStatus();
        uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);
    }


    // sx1262 choose packetType, user-interactive
    if (0) {
        sleep(10);
        if (helpFlag) {
          uartPrintf("\nSelect a packetType for the device\n");
          uartPrintf("0: PACKET_TYPE_LORA\n");
          uartPrintf("1: PACKET_TYPE_GFSK\n");
          uartPrintf("2: LoRA Sync Word = 0x3444\n");
          uartPrintf("3: LoRA Sync Word = 0x1424\n");
          helpFlag = 1;
        }
        uartPrintf("\n>> ");
        len = uartReceive(buffer); // This will block until we receive a newline

        switch(buffer[0]) {
            case '0': // LORA
                  setPacketType(PACKET_TYPE_LORA);
                  uartPrintf("Setting packet type to LORA...\n");
                  //status = getStatus();
                  uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);
                  break;
            case '1': // GFSK
                  setPacketType(PACKET_TYPE_GFSK);
                  uartPrintf("Setting packet type to GFSK...\n");
                  break;
            case '2': // set sync word to 0x3444 for Public Network
                  setLoraSyncWord(0x3444);
                  uartPrintf("Setting LoRA Sync Word to 0x3444...\n");
                  break;
            case '3': // Set to 0x1424 for Private Network
                  setLoraSyncWord(0x1424);
                  uartPrintf("Setting LoRA Sync Word to 0x1424...\n");
                  break;
            default:
                break;
        }
        uartPrintf("\nReadback:\n");
        packetType = getPacketType();
        uartPrintf("Packet Type: %x\n", packetType);
             
        syncword = getLoraSyncWord();
        uartPrintf("LoRA Sync World: %x\n", syncword);
    }

    // sx1262 Receive Test
    if (0) {
      if (helpFlag) {
          uartPrintf("\n\n\nSetting to standby mode...\n");
          setStandby(STANDBY_RC);
          
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
          status = getStatus();
          uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);

          uartPrintf("Setting interrupt mask...\n");
          setDioIrqParams();

          uartPrintf("Setting sync word...\n");
          setLoraSyncWord(0x1424);

          uartPrintf("Going to Rx mode...\n");
          setRx(0);
         
          helpFlag = 0;
      }

      uartPrintf("Press enter to get status info. Command status == 2 means we got a packet. \n>> ");

      len = uartReceive(buffer); // This will block until we receive a newline
      status = getStatus();
      uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);

      if (status.command_status == 2) {
          irqStatus = getIrqStatus();
          uartPrintf("Irq status: 0x%x\n", irqStatus);

          rxBufferStatus = getRxBufferStatus();
          //readBuffer(data, rxBufferStatus.RxStartBufferPointer, rxBufferStatus.PayloadLengthRx);
          //data[rxBufferStatus.PayloadLengthRx] = '\0';
          uartPrintf("Received this many bytes: %i\n", rxBufferStatus.PayloadLengthRx);
          //uartPrintf("Received packet starts at this address: %i\n", rxBufferStatus.RxStartBufferPointer);
          //uartPrintf("The contents is this: %s\n", data);
      }
      }


      // sx1262 Transmit Test
      if (1) {
        if (helpFlag) {

            status = getStatus();
            uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);

            uartPrintf("Enter RF frequency setting\n>> ");
            len = uartReceive(buffer); // This will block until we receive a newline
            buffer[len] = '\0';
            freq = atol(buffer);
            uartPrintf("You entered: %lu\n", freq);



            setDIO2AsRfSwitchCtrl(1);

            uartPrintf("\n\n\nSetting to standby mode...\n");
            setStandby(STANDBY_RC);
          
            
            //uartPrintf("Setting to LoRA packet type...\n");
            //setPacketType(0);

            uartPrintf("Setting RF frequency to %lu MHz...\n", freq);
            setRfFrequency(freq);

            uartPrintf("Setting Tx params...\n");
            //setTxParams(0xef, 0x04);

            //sleep(1);
            //uartPrintf("Setting buffer base address...\n");
            //setBufferBaseAddress(0,10);

            //uartPrintf("Writing to buffer, length %i\n", strlen("Hello world"));
            //writeBuffer("Hello world", 0, strlen("Hello world"));

            //uartPrintf("Setting modulation parameters...\n");
            //setModulationParams();

            //uartPrintf("Setting the packet params...\n");
            //setPacketParams();
         
                      
            uartPrintf("Going to Tx mode...\n");
            setTxContinuousWave();
            //helpFlag = 0;
      }
      sleep(100);
      status = getStatus();
      uartPrintf("Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);
      }

  }// end while loop
}

