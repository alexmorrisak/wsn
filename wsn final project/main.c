#include <msp430.h>
#include "myuart.h"
#include "myspi.h"
#include "miscellaneous.h"
#include "sx1262.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ADC.c"
#include "mytimer.h"

#define NSLAVES 2

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
    char deviceID = 1;
    unsigned int packetNumber = 0;
    int verbose = 0;
    unsigned long timestamp_coarse[NSLAVES];
    unsigned int timestamp_fine[NSLAVES];
    unsigned int temperature[NSLAVES];
    int deviceId[NSLAVES];
  

    unsigned long coarse, tstart, waitTime;
    unsigned int fine;
    int islave, nslaves;
    nslaves = NSLAVES;

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
    //setDioIrqParams(0xffff, 0xffff, 0, 0); // turn on all interrupts, map them to DIO1
    setDioIrqParams(IRQ_TXDONE | IRQ_RXDONE | IRQ_TIMEOUT, 0xffff, 0, 0); // turn on interrupts, map all to DIO1

    setLoraSyncWord(0x1424);
    getTime(&tstart, &fine);
     while (1) {
        getTime(&coarse, &fine);
        if (verbose) uartPrintf("Time (coarse, fine): %lu, %u\n", coarse, fine);

        if (verbose) {
          uartPrintf("Sending packet #%i\n", packetNumber);
        }

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
        buffer[9] = 0x00;
        buffer[10] = 0x00;
        buffer[11] = 0x00;
        buffer[12] = 0x00;
        buffer[13] = 0x00;
        buffer[14] = 0x00;
        buffer[15] = 0x00;
        writeBuffer(buffer, 0, 16);
        toggleLED();
        clearIrqStatus(0xffff);
        setTx(0); // transmit the packet, no timeout
        while (!radioInterruptFlag) {
            LPM0;
        }
        radioInterruptFlag = 0;
        sleep(1);
        
        for (islave=0; islave<nslaves; islave++) {
          if (verbose) uartPrintf("Receiving slave %i\n", islave);
          clearIrqStatus(0xffff);
          sleepUntil(tstart + 10*(islave+1)-1);
          getTime(&coarse, &fine);
          if (verbose) uartPrintf("Starting to receive at time (coarse, fine): %lu, %u\n", coarse, fine);
          setRx(3200); // 64 kHz clock ticks, 50 msec
          // wait for a radio packet or a timeout
          while (!radioInterruptFlag) {
              LPM0;
          }
          getTime(&coarse, &fine);
          if (verbose) uartPrintf("Time (coarse, fine): %lu, %u\n", coarse, fine);

          radioInterruptFlag = 0;
          irqStatus = getIrqStatus();
          if (verbose)  uartPrintf("Interrupt vector: 0x%04x\n", irqStatus);
          if (irqStatus == IRQ_RXDONE) {
              rssi = getPacketStatus();
              rxBufferStatus = getRxBufferStatus();
              readBuffer(data, rxBufferStatus.RxStartBufferPointer, rxBufferStatus.PayloadLengthRx);
              if (verbose) {
              uartPrintf("Received a packet. length: %i, RSSI: %.1f dBm, data: ", rxBufferStatus.PayloadLengthRx, -1*rssi/2.);
                  for (i=0; i<rxBufferStatus.PayloadLengthRx; i++) {
                      uartPrintf("%02x", data[i]);
                  }
                  uartPrintf("\n");
              }
              timestamp_coarse[islave] = 0;
              timestamp_coarse[islave] += (unsigned long) (data[3]) << 24;
              timestamp_coarse[islave] += (unsigned long) (data[4]) << 16;
              timestamp_coarse[islave] += (unsigned long) (data[5]) << 8;
              timestamp_coarse[islave] += (unsigned long) (data[6]) << 0;
              timestamp_fine[islave] = 0;
              timestamp_fine[islave] += data[7] * 2^8;
              timestamp_fine[islave] += data[8] * 2^0;
              temperature[islave] = 0;
              temperature[islave] += data[9] << 8;
              temperature[islave] += data[10] << 0;
              deviceId[islave] = data[0];
           } else if (irqStatus == IRQ_TIMEOUT) {
              if (verbose) uartPrintf("Timeout occured\n");
              timestamp_coarse[islave] = 0;
              timestamp_fine[islave] = 0;
              temperature[islave] = 0;
           } else {
              if (verbose) uartPrintf("Unhandled event: %04x\n", irqStatus);
           }
        }

        uartPrintf("%i, %lu, %u, ", packetNumber, tstart, 0); // print parameters of the master
        for (islave=0; islave<nslaves; islave++) {
          uartPrintf("%i, %lu, %u, %u, ", deviceId[islave], timestamp_coarse[islave], timestamp_fine[islave], temperature[islave]); // print parameters of each slave
        }
        uartPrintf("\n");
              

        packetNumber++;
        tstart += 1*64;
        sleepUntil(tstart); // sleep until the next 1-second interval
     }
}
   