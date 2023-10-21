#include <msp430.h>
#include "myuart.h"
#include "myspi.h"
#include "miscellaneous.h"
#include "sx1262.h"
#include <stdio.h>
#include <string.h>


void main(void){
  int i, j, len;
  int helpFlag = 1;
  char buffer[256];
  char uartBuffer[256];
  struct Status status;
  char standbyMode = 0;
  char packetType = 0;
  int syncword = 0;
  
  IncrementVcore();
  IncrementVcore();
  IncrementVcore();

  initClock();
  initSPI();
  initSPI(); // need to init this a 2nd time, otherwise SPI fails to transmit the first char. WHY?
  initUART();
  _EINT();


  while (1) {

    // UART Loopback test
    if (0) {
      if (helpFlag) {
          sprintf(uartBuffer, "\n\n\nType whatever you want. The MCU will repeat it back to you 3x.\n");
          helpFlag = 0;
          uartTransmit(uartBuffer, strlen(uartBuffer));
      }

      sprintf(uartBuffer, "\n>> ");
      uartTransmit(uartBuffer, strlen(uartBuffer));
      len = uartReceive(buffer); // This will block until we receive a newline
      toggleLED();
      uartTransmit(buffer, len);
      uartTransmit(buffer, len);
      uartTransmit(buffer, len);
    }

    // SPI Loopback test
    if (0) {
      if (helpFlag) {
          sprintf(uartBuffer, "\n\n\nType whatever you want.\nThe MCU will run it through the SPI bus and repeat it back to you.\nDon't forget to connect the SPI MOSI to MISO.\n");
          helpFlag = 0;
          uartTransmit(uartBuffer, strlen(uartBuffer));
      }
      sprintf(uartBuffer, "\n>> ");
      uartTransmit(uartBuffer, strlen(uartBuffer));

      len = uartReceive(buffer); // This will block until we receive a newline
      toggleLED();
      spiTransmit(buffer, len);
      len = spiReceive(buffer);
      uartTransmit(buffer, len);
    }

    // SX1262 basic test
    if (0) {
        if (helpFlag) {
          sprintf(uartBuffer, "\n\n\nPress enter to get the status of the device\n");
          helpFlag = 0;
          uartTransmit(uartBuffer, strlen(uartBuffer));
        }
        len = uartReceive(buffer); // This will block until we receive a newline
        buffer[0] = 0xC0;
        buffer[1] = 0x00;
        spiTransmit(buffer, 2);
        len = spiReceive(buffer);
        sprintf(uartBuffer, "Received: %x, %x\n", buffer[0], buffer[1]);
        uartTransmit(uartBuffer, strlen(uartBuffer));
    }

    // sx1262 step through states test, user-interactive
    if (0) {
        if (helpFlag) {
          sprintf(uartBuffer, "\nSelect a state for the device\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          sprintf(uartBuffer, "2: STANDBY_RC\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          sprintf(uartBuffer, "3: STANDBY_XTAL\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          sprintf(uartBuffer, "4: FS\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          sprintf(uartBuffer, "5: RX\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          sprintf(uartBuffer, "6: TX\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          helpFlag = 1;
        }
        sprintf(uartBuffer, "\n>> ");
        uartTransmit(uartBuffer, strlen(uartBuffer));
        len = uartReceive(buffer); // This will block until we receive a newline

        switch(buffer[0]) {
            case '2': // STANDBY_RC
                  standbyMode = 0;
                  setStandby(standbyMode);
                  sprintf(uartBuffer, "Setting mode to standby, mode %i...\n", standbyMode);
                  uartTransmit(uartBuffer, strlen(uartBuffer));
                  break;
            case '3': // STANDBY_XTAL
                  standbyMode = 1;
                  setStandby(standbyMode);
                  sprintf(uartBuffer, "Setting mode to standby, mode %i...\n", standbyMode);
                  uartTransmit(uartBuffer, strlen(uartBuffer));
                  break;
            case '4': // FS
                  setFs();
                  sprintf(uartBuffer, "Setting mode to frequency synthesis...\n");
                  uartTransmit(uartBuffer, strlen(uartBuffer));
                  break;
            case '5': // RX
                  setRx(0);
                  sprintf(uartBuffer, "Setting mode to receive...\n");
                  uartTransmit(uartBuffer, strlen(uartBuffer));
                  break;
            case '6': // TX
                    sprintf(uartBuffer, "Setting frequency to 915 MHz...\n");
                  uartTransmit(uartBuffer, strlen(uartBuffer));
                  setRfFrequency(915);
                  sprintf(uartBuffer, "Done setting frequency to 915 MHz (I hope)\n");
                  uartTransmit(uartBuffer, strlen(uartBuffer));
                  sleep(10);

                  setTx(0);
                  sprintf(uartBuffer, "Setting mode to transmit...\n");
                  uartTransmit(uartBuffer, strlen(uartBuffer));
                  break;
            default:
                break;
        }
        sprintf(uartBuffer, "\nReadback:\n");
        uartTransmit(uartBuffer, strlen(uartBuffer));
        status = getStatus();
        sprintf(uartBuffer, "Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);
        uartTransmit(uartBuffer, strlen(uartBuffer));
    }


    // sx1262 choose packetType, user-interactive
    if (0) {
        sleep(10);
        if (helpFlag) {
          sprintf(uartBuffer, "\nSelect a packetType for the device\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          sprintf(uartBuffer, "0: PACKET_TYPE_GFSK\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          sprintf(uartBuffer, "1: PACKET_TYPE_LORA\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          sprintf(uartBuffer, "2: LoRA Sync Word = 0x3444\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          sprintf(uartBuffer, "3: LoRA Sync Word = 0x1424\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          helpFlag = 1;
        }
        sprintf(uartBuffer, "\n>> ");
        uartTransmit(uartBuffer, strlen(uartBuffer));
        len = uartReceive(buffer); // This will block until we receive a newline

        switch(buffer[0]) {
            case '0': // GFSK
                  setPacketType(0);
                  sprintf(uartBuffer, "Setting packet type to GFSK...\n");
                  uartTransmit(uartBuffer, strlen(uartBuffer));
                  break;
            case '1': // LORA
                  setPacketType(1);
                  sprintf(uartBuffer, "Setting packet type to LoRA...\n");
                  uartTransmit(uartBuffer, strlen(uartBuffer));
                  break;
            case '2': // set sync word to 0x3444 for Public Network
                  setLoraSyncWord(0x3444);
                  sprintf(uartBuffer, "Setting LoRA Sync Word to 0x3444...\n");
                  uartTransmit(uartBuffer, strlen(uartBuffer));
                  break;
            case '3': // Set to 0x1424 for Private Network
                  setLoraSyncWord(0x1424);
                  sprintf(uartBuffer, "Setting LoRA Sync Word to 0x1424...\n");
                  uartTransmit(uartBuffer, strlen(uartBuffer));
                  break;
            default:
                break;
        }
        sprintf(uartBuffer, "\nReadback:\n");
        uartTransmit(uartBuffer, strlen(uartBuffer));
        packetType = getPacketType();
        sprintf(uartBuffer, "Packet Type: %x\n", packetType);
        uartTransmit(uartBuffer, strlen(uartBuffer));
             
        syncword = getLoraSyncWord();
        sprintf(uartBuffer, "LoRA Sync World: %x\n", syncword);
        uartTransmit(uartBuffer, strlen(uartBuffer));

    }

    // sx1262 Receive Test
    if (1) {
      if (helpFlag) {
          sprintf(uartBuffer, "\n\n\nSetting to standby mode...\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          setStandby(1);
          
          sprintf(uartBuffer, "Setting to LoRA packet type...\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          setPacketType(0);

          sprintf(uartBuffer, "Setting RF frequency to 915 MHz...\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          setRfFrequency(915);


          sprintf(uartBuffer, "Setting modulation parameters...\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          setModulationParams();


          sprintf(uartBuffer, "Setting packet parameters...\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          //setThePacketParams();
/*

          sprintf(uartBuffer, "\n\n\nGoing to Rx mode...\n");
          uartTransmit(uartBuffer, strlen(uartBuffer));
          setRx(0);
*/
         
          helpFlag = 0;
      }

      sprintf(uartBuffer, "Press enter to get status info. Command status == 2 means we got a packet. \n>> ");
      uartTransmit(uartBuffer, strlen(uartBuffer));

      len = uartReceive(buffer); // This will block until we receive a newline
      status = getStatus();
      sprintf(uartBuffer, "Chip mode: %x\nCommand status: %x\n", status.chip_mode, status.command_status);
      uartTransmit(uartBuffer, strlen(uartBuffer));    }


  }// end while loop
}

