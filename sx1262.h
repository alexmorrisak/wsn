#ifndef SX1262_H
#define SX1262_H

#include "myspi.h"

// getStatus()
struct Status {
    int chip_mode;
    // 0x0: unused
    // RFU
    // 0x2: STBY_RC
    // 0x3: STBY_XOSC
    // 0x4: FS
    // 0x5: RX
    // 0x6: TX


    int command_status;
    // 0x0: Reserved
    // RFU
    // 0x2: Data is available to host
    // 0x3: Command timeout
    // 0x4: Command processing error
    // 0x5: Failure to execute command
    // 0x6: Command Tx done
};
struct Status getStatus(void) {
    int len;
    struct Status status;
    char buff[2] = {0xc0, 0x00};
    spiTransmit(buff, 2);
    len = spiReceive(buff);
    status.chip_mode = (buff[1] >> 4) & 0x7;
    status.command_status = (buff[1] >> 1) & 0x3;
    return status;
}


// setStandby(standbyMode)
// standbyMode 0 = STDBY_RC
// standbyMode 1 = STDBY_XOSC
void setStandby(char standbyMode) {
    int len;
    char buff[2];
    buff[0] = 0x80;
    buff[1] = standbyMode;
    spiTransmit(buff, 2);
    len = spiReceive(buff);
}

// setRx(timeout)
void setRx(unsigned long timeout) {
    int len;
    char buff[4];
    buff[0] = 0x82;
    buff[1] = timeout >> 16;
    buff[2] = timeout >> 8;
    buff[3] = timeout >> 0;
    spiTransmit(buff, 4);
    len = spiReceive(buff);
}

// setTx(timeout)
void setTx(unsigned long timeout) {
    int len;
    char buff[4];
    buff[0] = 0x83;
    buff[1] = timeout >> 16;
    buff[2] = timeout >> 8;
    buff[3] = timeout >> 0;
    spiTransmit(buff, 4);
    len = spiReceive(buff);
}

// setFs(timeout)
void setFs(void) {
    int len;
    char buff[1];
    buff[0] = 0xC1;
    spiTransmit(buff, 1);
    len = spiReceive(buff);
}

// setPacketType(char packetType)
// packetType 0 = LoRA
// packetType 1 = GFSK
void setPacketType(char packetType) {
    int len;
    char buff[2];
    buff[0] = 0x8A;
    buff[1] = packetType;
    spiTransmit(buff, 2);
    len = spiReceive(buff);
}
// getPacketType()
// returns the packetType:
// packetType 0 = LoRA
// packetType 1 = GFSK
int getPacketType(void) {
    int len;
    char buff[3];
    buff[0] = 0x11;
    buff[1] = 0x00;
    buff[2] = 0x00;
    spiTransmit(buff, 3);
    len = spiReceive(buff);
    return buff[2];
}


// setRfFrequency(int frequency)
void setRfFrequency(int frequency_mhz) {
    int len;
    char buff[5];
    unsigned long bitval = 2^25*frequency_mhz/32;
    buff[0] = 0x86;
    buff[1] = (bitval >> 24) & 0xff;
    buff[2] = (bitval >> 16) & 0xff;
    buff[3] = (bitval >> 8) & 0xff;
    buff[4] = (bitval >> 0) & 0xff;
    spiTransmit(buff, 5);
    len = spiReceive(buff);
}

void setBufferBaseAddress(char txAddr, char rxAddr){
    int len;
    char buff[3];
    buff[0] = 0x8F;
    buff[1] = txAddr;
    buff[2] = rxAddr;
    spiTransmit(buff, 3);
    len = spiReceive(buff);
}

void setModulationParams(void){
    int len;
    char buff[4];
    buff[0] = 0x8B;
    buff[1] = 0x07; //spreading factor of 7 (dev kit default)
    buff[2] = 0x06;//corresponds to 500 kHz bandwidth (dev kit default)
    buff[3] = 0x01; //coding rate of 4/5 (dev kit default)
    spiTransmit(buff, 4);
    len = spiReceive(buff);
}

#endif
