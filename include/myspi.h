#ifndef MYSPI_H
#define MYSPI_H


char spiTxBuffer[256];
char spiRxBuffer[256];
char spiMsgLength = 0;
char spiTxPtr = 0;
char spiRxPtr = 0;
char spiRxDone = 0;

void spiTransmit(char *txbuffer, int length) {
    int i;
    UCB0IE &= ~UCTXIE; // disable Tx interrupts while in this section
    spiMsgLength = length;
    for (i=0; i<length; i++){
      spiTxBuffer[i] = txbuffer[i];
    }
    spiRxDone = 0;
    spiTxPtr = 1; // reset the pointer
    spiRxPtr = 0;
    P3OUT &= ~BIT0; // SS goes low
    while(UCTXIFG == 0); // check that buffer is empty
    UCB0TXBUF = spiTxBuffer[0]; // Send the first byte
    UCB0IE |= UCTXIE; // re-enable Tx interrupts
}


int spiReceive(char *rxbuffer) {
    int i, msgLength;
    while(spiRxDone == 0) {
        LPM0;
    }
    for (i=0; i<spiMsgLength; i++){
        rxbuffer[i] = spiRxBuffer[i];
    }
    return spiMsgLength;
}

void initSPI(void) {   
    P3SEL |= BIT3; // Clock
    P3SEL |= BIT2; // MISO
    P3SEL |= BIT1; // MOSI
    P3SEL &= ~BIT0; // Use this as GPIO pin for SS
    P3DIR |= BIT3 | BIT1 | BIT0;
    P3OUT |= BIT0; // Set the SS high to start
    

    UCB0CTL1 = UCSWRST;  // **Put  module in reset**
    UCB0CTL1 = UCSSEL__SMCLK;

    UCB0BRW = 80; // BRCLK / UCBRx = UCxCLK
                  // 8MHz  / 80    = 100kHz

    UCB0CTL0 |=  UCCKPH;  // Clock phase = 1. Note: the TI convention seems to be opposite the Motorola convention
    //UCB0CTL0 |= UCCKPL; // Uncomment to set clock polarity = 1
    UCB0CTL0 |= UCMSB;  // MSB first
    UCB0CTL0 &= ~UC7BIT; // 8-bit mode, not 7-bit mode
    UCB0CTL0 |= UCSYNC; // Synchronous mode
    UCB0CTL0 |= UCMST; // SPI master
                                                            
    UCB0CTL1 &= ~UCSWRST;                  // **Initialize eUSCI module**
    UCB0IE |= UCRXIE | UCTXIE;          // Enable interrupts
}


void spiInterrupt(void) __interrupt [USCI_B0_VECTOR]{
     switch (UCB0IV) {
        case 0x2: // Rx interrupt
            spiRxBuffer[spiRxPtr] = UCB0RXBUF;
            spiRxPtr++;
            if (spiRxPtr == spiMsgLength && spiRxDone == 0) { // we have received the full message. wake up
                spiRxDone = 1;
                // Set the SS high to end the transmission.
                // Note: this should be done on the Rx side, because at this point we know
                // that the transmission has actually completed. If we try to do it on the Tx side,
                // we only know that the peripheral can accept another byte, not that it has completed
                // transmission of the previous byte
                P3OUT |= BIT0;
                LPM0_EXIT;
            }
          break;
        case 0x4: // Tx interrupt
            if (spiTxPtr != spiMsgLength) { // there are bytes in the buffer that need to be sent
                while(UCTXIFG == 0);
                UCB0TXBUF = spiTxBuffer[spiTxPtr];
                spiTxPtr++;
            } 
            break;
        default:
            break;
     }   
     
}

#endif

