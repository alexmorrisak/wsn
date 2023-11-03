#ifndef MYUART_H
#define MYUART_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <miscellaneous.h>

#define UART_TX_BUFFER_SIZE 256
#define UART_RX_BUFFER_SIZE 256
// use uint8 (256 length) get rid of modulo arithmetic
char uartTxBuffer[UART_TX_BUFFER_SIZE];
unsigned char uartTxHead = 0;
unsigned char uartTxTail = 0;
char uartRxBuffer[UART_RX_BUFFER_SIZE];
unsigned char uartRxHead = 0;
unsigned char uartRxTail = 0;
unsigned int uartRxMsgAvailable = 0;
char uartPrintfBuffer[256];
uartTxActive = 0;

void initUART(void) {
  P5SEL |= BIT7 + BIT6;//SETUP PINS
  P5DIR |= BIT6; //5.6=TX
  P5DIR &= ~BIT7;//5.7=RX
  UCA1CTL0 = 0b11010000;//EVEN PARITY, LSB FIRST, 7BIT DATA, 1 STOP BIT, UART ASYNCHRONOUS
  UCA1CTL1 |= 0xC0; //USE SMCLK, KEEP IN RESET MODE
  UCA1MCTL |= UCOS16;//USE OVERSAMPLING MODE
  UCA1BR0 = 27; //SET BAUD RATE: N=442.88. 38400 baud
  UCA1MCTL |= UCBRF_11 + UCBRS_7;//SET UCBRFx AND UCBRSx
  UCA1CTL1 &= ~UCSWRST; //STOP RESET
  UCA1IE |= UCTXIE; //ENABLE INTERRUPTS FOR TRANSMIT
  UCA1IE |= UCRXIE; //ENABLE INTERRUPTS FOR RECEIVE
}

int uartReceive(char *buffer) {
    int i = 0;
    while(uartRxMsgAvailable == 0) {
        LPM0;
    }
    while(uartRxTail != uartRxHead) {
        buffer[i] = uartRxBuffer[uartRxTail];
        uartRxTail++;
        i++;
    }
    uartRxMsgAvailable--;
    return i;
}

void uartTransmit(char *buffer, int length){
    int i = 0;
    int startTxFlag = 0;
    unsigned char newHead;

    newHead = uartTxHead;

    for (i=0; i<length; i++) {
        while ((newHead+1) == uartTxTail) {
        toggleLED();
        }; // buffer is full. wait
        uartTxBuffer[newHead] = buffer[i];
        newHead++;
    }

    //UCA1IE &= ~UCTXIE; //DISABLE INTERRUPTS IN THE CRITICAL SECTION

    if (!uartTxActive) { // no transmission active. need to initiate
  //      while (UCTXIFG == 0) {}; // wait until the buffer is empty
        UCA1TXBUF = uartTxBuffer[uartTxTail]; // send the first byte
        uartTxTail++;// = (uartTxTail + 1) % UART_TX_BUFFER_SIZE;
        uartTxActive = 1;
    }
    uartTxHead = newHead;

    //UCA1IE |= UCTXIE; //RE ENABLE TX INTERRUPT

}

int uartPrintf(char *format, ...) {
   va_list aptr;
   int ret;

   va_start(aptr, format);
   ret = vsprintf(uartPrintfBuffer, format, aptr);
   va_end(aptr);

   uartTransmit(uartPrintfBuffer, strlen(uartPrintfBuffer));

   return(ret);
}

void uartInterrupt(void) __interrupt [USCI_A1_VECTOR]{
    int wakeFlag = 0;
 
    switch (UCA1IV) {
        case 0x2: // Rx interrupt
            uartRxBuffer[uartRxHead] = UCA1RXBUF;
            if (uartRxBuffer[uartRxHead] == '\n') {
                wakeFlag = 1;
                uartRxMsgAvailable++;
            }
            uartRxHead++;// = (uartRxHead + 1) % UART_RX_BUFFER_SIZE;
            if (wakeFlag) {
                LPM0_EXIT;
            }
          break;
        case 0x4: // Tx interrupt
            if (uartTxTail != uartTxHead) { // there are bytes in the buffer that need to be sent
                UCA1TXBUF = uartTxBuffer[uartTxTail];
                uartTxTail++;
            } else {
                uartTxActive = 0;
            }
            
            break;
        default:
            break;
     }   
    
}

#endif
