#ifndef MYTIMER_H
#define MYTIMER_H

unsigned long myTimerCoarse = 0; // global counter for coarse clock ticks
unsigned long myTimerFine = 0; // global flag for fine clock ticks

// initialize the timer
void initTimer(void) {
    TA1CTL = TASSEL_2 + MC_1 + ID_3 + TAIE; // divide by 8, count in the UP mode, use SMCLK, enable interrupts
    TA1CCR0 = 33151; // 15.625 msec per coarse tick
}

// get the current time in both coarse and fine clock ticks
void getTime(unsigned long *coarse, unsigned int* fine) {
    *coarse = myTimerCoarse;
    *fine = TA1R;
}

// device will sleep until the specified count in coarse clock ticks
void sleepUntil(unsigned long coarse) {
    while (myTimerCoarse < coarse) LPM0;
}

// device will sleep until the specified count in coarse and fine clock ticks
void sleepUntilPrecise(unsigned long coarse, unsigned int fine) {
    while (myTimerCoarse < coarse-1) LPM0; // sleep until 1 coarse tick prior to wake up
    myTimerFine = 0;
    TA1CCR1 = fine;
    TA1CCTL1 = 0x10;
    while (TA1R < fine || myTimerCoarse < coarse) LPM0;
    TA1CCTL1 = 0;
}

// interrupt service routine. gets triggered when the timer counter rolls over
void timerInterrupt(void) __interrupt [TIMER1_A1_VECTOR]{
    int wakeflag = 0;
    switch (TA1IV) {
        case 0x2: // fine precision
          myTimerFine++;
          wakeflag = 1;
          break;
        case 0xe: // overflow
         myTimerCoarse++;
         wakeflag = 1;
         break;
        default:
            break;
     }    
     if (wakeflag) LPM0_EXIT;
}

#endif
