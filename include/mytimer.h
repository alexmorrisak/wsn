#ifndef MYTIMER_H
#define MYTIMER_H

unsigned long myTimerCoarse = 0; // global counter for coarse clock ticks

// initialize the timer
void initTimer(void) {
    TA1CTL = TASSEL_2 + MC_2 + ID_3 + TAIE; // divide by 8, count continuously upwards, use SMCLK, enable interrupts
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

// interrupt service routine. gets triggered when the timer counter rolls over
void timerInterrupt(void) __interrupt [TIMER1_A1_VECTOR]{
    int wakeflag = 0;
    switch (TA1IV) {
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
