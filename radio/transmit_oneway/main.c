#include <msp430.h>
#include "myuart.h"
#include "myspi.h"
#include "miscellaneous.h"
#include "sx1262.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



void main(void){
    int len;
    char ipacket = 0;
    char npackets = 100;
    char buffer[256];
    struct Status status;
    int irqStatus;
  
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
    setDioIrqParams(IRQ_TXDONE, 0xff, 0, 0); // turn on RXDONE interrupts, map all to DIO1

    setLoraSyncWord(0x1424);

    // Set the data for the data payload
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 0;
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
    writeBuffer(buffer, 0, 16);

    while(1) {
        uartPrintf("How many packets (1-255) [Default=10]?\n");
        uartPrintf(">> ");
        len = uartReceive(buffer);
        buffer[len] = '\0';
        npackets = atoi(buffer);
        if (npackets == 0) {
            npackets = 10;
        }

        clearIrqStatus(0xff);
        for (ipacket=0; ipacket<npackets; ipacket++) {
            uartPrintf("Sending %i of %i\n", ipacket+1, npackets);

            writeBuffer(&ipacket, 3, 1);

            setTx(0); // transmit the packet
            while (!radioInterruptFlag) {
                LPM0;
            }
            radioInterruptFlag = 0;
            clearIrqStatus(IRQ_TXDONE);
            sleep(1);
        }// end while loop
    }
}

