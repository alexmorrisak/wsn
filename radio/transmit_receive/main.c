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

        // wait for either a UART message, or a radio interrupt packet
        while (!uartRxMsgAvailable && !radioInterruptFlag) {
            LPM0;
        }
        if (uartRxMsgAvailable) { // Got something on the UART
            len = uartReceive(data);
            writeBuffer(data, 0, len);
            clearIrqStatus(0xff);
            setTx(0); // transmit the packet

            while(!radioInterruptFlag) {
               LPM0;
            }
            radioInterruptFlag = 0;
            irqStatus = getIrqStatus();
            uartPrintf("Interrupt vector: 0x%02x\n", irqStatus);
            clearIrqStatus(irqStatus);
        } else {
            radioInterruptFlag = 0;
            irqStatus = getIrqStatus();
            uartPrintf("Interrupt vector: 0x%02x\n", irqStatus);
            clearIrqStatus(irqStatus);
            rssi = getPacketStatus();
            rxBufferStatus = getRxBufferStatus();
            readBuffer(data, rxBufferStatus.RxStartBufferPointer, rxBufferStatus.PayloadLengthRx);
            uartPrintf("Received a packet. length: %i, RSSI: %.1f dBm, data: ", rxBufferStatus.PayloadLengthRx, -1*rssi/2.);
            for (i=0; i<rxBufferStatus.PayloadLengthRx; i++) {
                uartPrintf("%02x", data[i]);
            }
            uartPrintf("\n");
            data[rxBufferStatus.PayloadLengthRx] = '\0';
            //uartPrintf("In ASCII format: %s\n", data);
            //uartPrintf("In ASCII format: ");
            //for (i=0; i<rxBufferStatus.PayloadLengthRx; i++) {
            //    uartPrintf("%c", data[i]);
            //}
            //uartPrintf("\n");
        }
    }// end while loop
}

