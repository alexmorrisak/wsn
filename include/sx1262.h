#ifndef SX1262_H
#define SX1262_H

#include "myspi.h"

int radioInterruptFlag = 0;

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
#define STANDBY_RC 0
#define STANDBY_XOSC 1
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

// setTxContinuousWave
void setTxContinuousWave(void) {
    char buff[1];
    buff[0] = 0xd1;
    spiTransmit(buff, 1);
    spiReceive(buff);
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
// packetType 0 = GFSK
// packetType 1 = LoRA
#define PACKET_TYPE_GFSK 0
#define PACKET_TYPE_LORA 1
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
void setRfFrequency(unsigned long frequency_mhz) {
    char buff[5];
    // frequency = bitvalue * F_XTAL / 2^25;
    // frequency = bitvalue * 32e6 / 2^25;
    // bitvalue = frequency_hz * 2^25 / 32e6;
    // bitvalue = frequency_mhz * 2^25 / 32;
    // bitvalue = frequency_mhz * 2^20;
    // E.g.: 915 Mhz yields 0x39300000
    unsigned long bitval;
    bitval = (frequency_mhz << 20);
    buff[0] = 0x86;
    buff[1] = (bitval >> 24) & 0xff;
    buff[2] = (bitval >> 16) & 0xff;
    buff[3] = (bitval >> 8) & 0xff;
    buff[4] = (bitval >> 0) & 0xff;
    spiTransmit(buff, 5);
    spiReceive(buff);
}

/*
This command sets the base addresses in the data buffer in all modes of operations for the packet handing operation in TX
and RX mode. The usage and definition of those parameters are described in the different packet type sections.
*/
void setBufferBaseAddress(char txAddr, char rxAddr){
    int len;
    char buff[3];
    buff[0] = 0x8F;
    buff[1] = txAddr;
    buff[2] = rxAddr;
    spiTransmit(buff, 3);
    len = spiReceive(buff);
}

/*
The command SetModulationParams(...) is used to configure the modulation parameters of the radio. Depending on the
packet type selected prior to calling this function, the parameters will be interpreted differently by the chip. 
*/
void setModulationParams(void){
    char buff[9];
    buff[0] = 0x8B;
    buff[1] = 0x07; //spreading factor of 7 (dev kit default)
    buff[2] = 0x06;//corresponds to 500 kHz bandwidth (dev kit default)
    buff[3] = 0x01; //coding rate of 4/5 (dev kit default)
    buff[4] = 0x00;
    buff[5] = 0x00;
    buff[6] = 0x00;
    buff[7] = 0x00;
    buff[8] = 0x00;
    spiTransmit(buff, 9);
    spiReceive(buff);
}

/*
void setDioIrqParams(void) {
    char buff[9];
    buff[0] = 0x08;
    buff[1] = 0xff;
    buff[2] = 0xff;
    buff[3] = 0x00;
    buff[4] = 0x00;
    buff[5] = 0x00;
    buff[6] = 0x00;
    buff[7] = 0x00;
    buff[8] = 0x00;
    spiTransmit(buff, 9);
    spiReceive(buff);
}
*/

#define IRQ_TXDONE 0x1
#define IRQ_RXDONE 0x2
#define IRQ_TIMEOUT 0x10
/* This command is used to set the IRQ flag. */
void setDioIrqParams(int irqMask, int dio1Mask, int dio2Mask, int dio3Mask) {
    char buff[9];
    buff[0] = 0x08;
    buff[1] = (irqMask >> 8) & 0xff;
    buff[2] = (irqMask >> 0) & 0xff;;
    buff[3] = (dio1Mask >> 8) & 0xff;;
    buff[4] = (dio1Mask >> 0) & 0xff;;
    buff[5] = (dio2Mask >> 8) & 0xff;;
    buff[6] = (dio2Mask >> 0) & 0xff;;
    buff[7] = (dio3Mask >> 8) & 0xff;;
    buff[8] = (dio3Mask >> 0) & 0xff;;
    spiTransmit(buff, 9);
    spiReceive(buff);
}

/*This command is used to set the parameters of the packet handling block.*/
void setPacketParams(void) {
    //     unsigned int preambleLength = 65535;
    unsigned int preambleLength = 12;
    char headerType = 0; // 0x00 = variable (explicit). 0x01 = fixed (implicit)
    char payloadLength = 16; // length in bytes. 0x00 to 0xff
    char CRCType = 1; // 0=off, 1=On
    char InvertIQ = 0; // 0=standard, 1=inverted
    char buff[10];
    
    buff[0] = 0x8C;
    buff[1] = (preambleLength >> 8) & 0xff;
    buff[2] = (preambleLength >> 0) & 0xff;
    buff[3] = headerType; // headerType. 0=variable, 1=fixed
    buff[4] = payloadLength; // payloadLength, in bytes. 0x00 to 0xff
    buff[5] = CRCType; // CRC type, 0=off, 1=on
    buff[6] = InvertIQ; // invertIQ. 0=standard, 1=inverted
    buff[7] = 0x00;
    buff[8] = 0x00;
    buff[9] = 0x00;
    spiTransmit(buff, 10);
    spiReceive(buff);
  }

/*
The command WriteRegister(...) allows writing a block of bytes in a data memory space starting at a specific address. The
address is auto incremented after each data byte so that data is stored in contiguous memory locations. The SPI data
transfer is described in the following table
*/
void writeRegister(char *buffer, int address, int length) {
    char buff[64];
    int i, len;
    buff[0] = 0x0D; // Write register command
    buff[1] = (address >> 8) & 0xff;// address[15:8]
    buff[2] = (address >> 0) & 0xff;// address[7:0]
    for (i=0;i<length;i++) {
        buff[3+i] = buffer[i]; // data to be written to register(s)
    }
    spiTransmit(buff, length+3);
    len = spiReceive(buff);
}


/*
The command ReadRegister(...) allows reading a block of data starting at a given address. The address is auto-incremented
after each byte. The SPI data transfer is described in Table 13-25. Note that the host has to send an NOP after sending the 2
bytes of address to start receiving data bytes on the next NOP sent.
*/
void readRegister(char *data, int address, int length) {
    char buff[64];
    int i;
    buff[0] = 0x1D; // Read register command
    buff[1] = (address >> 8) & 0xff;// address[15:8]
    buff[2] = (address >> 0) & 0xff;// address[7:0]
    buff[3] = 0x00; // NOP
    for (i=0;i<length;i++) {
        buff[i+4] = 0x00; // NOP
    }
    //spiTransmit(buff, length+4);
    spiTransmit(buff,length+4);
    //spiReceive(buff);
    spiReceive(buff);
    for (i=0;i<length;i++) {
        data[i] = buff[i+4]; // Data contents of the register(s)
    }
}

/*
This function is used to store data payload to be transmitted. The address is auto-incremented; when it exceeds the value
of 255 it is wrapped back to 0 due to the circular nature of the data buffer. The address starts with an offset set as a
parameter of the function. Table 13-26 describes the SPI data transfer.
*/
void writeBuffer(char *data, int offset, int length) {
    char buff[64];
    int i, len;
    buff[0] = 0x0E; // Write register command
    buff[1] = offset & 0xff; // offset within the data buffer
    for (i=0;i<length;i++) { // copy the data into a contiguous buffer, to be sent over spi
        buff[i+2] = data[i]; // data to be written to register(s)
    }
    spiTransmit(buff, length+2);
    len = spiReceive(buff);
}

/*
This function allows reading (n-3) bytes of payload received starting at offset. Note that the NOP must be sent after sending
the offset.
*/
void readBuffer(char *data, int offset, int length) {
    char buff[64];
    int i, len;
    buff[0] = 0x1E; // Read register command
    buff[1] = offset & 0xff;// offset address
    buff[2] = 0x00; // NOP
    for (i=0;i<length;i++) {
        buff[i+3] = 0x00; // NOP
    }
    spiTransmit(buff, length+3);
    len = spiReceive(buff);
    for (i=0;i<len;i++) {
        data[i] = buff[i+3]; // Data contents of the register(s)
    }
}

// Differentiate the LoRa® signal for Public or Private Network
// Set to 0x3444 for Public Network
// Set to 0x1424 for Private Network
void setLoraSyncWord(int syncword) {
    int len;
    char buff[2];
    buff[0] = (syncword >> 8) & 0xff; //MSB of sync word
    buff[1] = (syncword >> 0) & 0xff; //LSB of sync word
    writeRegister(buff, 0x0740, 2);
}

// Reads the register location corresponding to the LoRA sync word
int getLoraSyncWord(void) {
    unsigned int syncword = 0;
    char buff[2];
    readRegister(buff, 0x0740, 2);
    syncword += (buff[0] << 8);
    syncword += (buff[1] << 0);
    return syncword;
}

// RxBufferStatus()
struct RxBufferStatus {
    int PayloadLengthRx;
    int RxStartBufferPointer;
};

// This command returns the length of the last received packet (PayloadLengthRx)
// and the address of the first byte received (RxStartBufferPointer).
// It is applicable to all modems.
// The address is an offset relative to the first byte of the data buffer.
struct RxBufferStatus getRxBufferStatus(void) {
    struct RxBufferStatus rbstatus;
    char buff[4];
    buff[0] = 0x13;
    buff[1] = 0x00;
    buff[2] = 0x00;
    buff[3] = 0x00;
    spiTransmit(buff, 4);
    spiReceive(buff);
    rbstatus.PayloadLengthRx = buff[2];
    rbstatus.RxStartBufferPointer = buff[3];
    return rbstatus;
}

// This command sets the TX output power by using the parameter power
// and the TX ramping time by using the parameter
/*
The output power is defined as power in dBm in a range of
• - 17 (0xEF) to +14 (0x0E) dBm by step of 1 dB if low power PA is selected
• - 9 (0xF7) to +22 (0x16) dBm by step of 1 dB if high power PA is selected
Selection between high power PA and low power PA is done with the command SetPaConfig and the parameter deviceSel.
By default low power PA and +14 dBm are set.
The power ramp time is defined by the parameter RampTime as defined in the following table:
0x00 thru 0x07
*/
void setTxParams(char power, char rampTime) {
    char buff[3];
    buff[0] = 0x8E;
    buff[1] = power;
    buff[2] = rampTime;
    spiTransmit(buff, 3);
    spiReceive(buff);
}


/*
SetPaConfig is the command which is used to differentiate the SX1261 from the SX1262. When using this command, the
user selects the PA to be used by the device as well as its configuration.
*/
void setPaConfig(char paDutyCycle, char hpMax, char deviceSel) {
    char buff[5];
    buff[0] = 0x95;
    buff[1] = paDutyCycle;
    buff[2] = hpMax;
    buff[3] = deviceSel;
    buff[4] = 0x01; // reserved, always 1
    spiTransmit(buff, 2);
    spiReceive(buff);
}

/*
This command is used to configure DIO2 so that it can be used to control an external RF switch.
1 = enable
0 = disable
*/
void setDIO2AsRfSwitchCtrl(char enable) {
    char buff[2];
    buff[0] = 0x9D;
    buff[1] = enable;
    spiTransmit(buff, 2);
    spiReceive(buff);
}

//This command returns the value of the IRQ register. 
int getIrqStatus(void){
    char buff[4];
    int irqStatus = 0;
    buff[0] = 0x12;
    buff[1] = 0x00;
    buff[2] = 0x00;
    buff[3] = 0x00;
    spiTransmit(buff, 4);
    spiReceive(buff);
    irqStatus |= (buff[2] << 8);
    irqStatus |= (buff[3] << 0);
    return irqStatus;
}


// This command clears an IRQ flag in the IRQ register. 
void clearIrqStatus(int ClearIrqParam) {
    char buff[3];
    buff[0] = 0x02;
    buff[1] = (ClearIrqParam >> 8) & 0xff;
    buff[2] = (ClearIrqParam >> 0) & 0xff;
    spiTransmit(buff, 3);
    spiReceive(buff);
}


char getPacketStatus(void) {
    char buff[5];
    buff[0] = 0x14;
    buff[1] = 0x00;
    buff[2] = 0x00;
    buff[3] = 0x00;
    buff[4] = 0x00;
    spiTransmit(buff, 5);
    spiReceive(buff);
    return buff[4];
}

void initInterruptPin(void) {
    P1DIR &= ~BIT3;//USE P1.3 IN RF3 FOR TX/RX INTERRUPT
    P1SEL &= ~BIT3;//SET PIN TO IO MODE
    P1IE |= BIT3;//ENABLE INTERRUPT FOR P1.3
    P1IES &= ~BIT3; //RISING EDGE TRIGGER
    P1IFG = 0;
}

void waitForInterrupt(void) {
    radioInterruptFlag = 0;
    if (getIrqStatus() > 0) return;
    while (!radioInterruptFlag) {
        LPM0; // wait for DIO interrupt from sx1262
    }
}

void radioInterrupt(void) __interrupt [PORT1_VECTOR]{
    P1IFG = 0;
    radioInterruptFlag=1;
    LPM0_EXIT;
}



#endif
