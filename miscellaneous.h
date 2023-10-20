#ifndef MISCELLANEOUS_H
#define MISCELLANEOUS_H


int IncrementVcore(void)
{

  unsigned int  PMMRIE_backup,SVSMHCTL_backup;
  unsigned short   old_level; 

  // read the previous Vcore level 
  old_level = PMMCTL0 & (PMMCOREV_3); 

  if (old_level < PMMCOREV_3) {
    // Enable write access to PMM registers
    PMMCTL0_H = 0xA5;
    
    // SAVE the previous content of PMM Reset and Interrupt Enable (PMMRIE) register
    PMMRIE_backup = PMMRIE;
    
    // Disable the following interrupts:
    //  high- and low-side "delay expired" interrupt
    //  SVM low-side (Vcore) voltage level reached interrupt (SVMLVLRIE)
    //  SVM high-side (DVcc) voltage level reached interrupt (SVMHVLRIE)
    //  SVM high-side (DVcc) voltage level reached POR interrupt (SVMHVLRPE) 
    //    - there will be a POR... 
    PMMRIE &= ~(SVSMHDLYIE | SVSMLDLYIE | SVMLVLRIE | SVMHVLRIE | SVMHVLRPE);
    
    // SAVE the previous content of (SVSMHCTL) register
    SVSMHCTL_backup = SVSMHCTL;
    
    // Clear the following flags: 
    //  SVM high-side interrupt flag
    //  SVS and SVM high-side delay expired interrupt flag
    PMMIFG &= ~(SVMHIFG | SVSMHDLYIFG);
    
    // Set SVM high-side to a new level and check if a VCore increase is possible
    //  Enable high-side SVM
    //  Enable high-side SVS
    //  Set the new voltage level for DVcc
    SVSMHCTL = SVMHE | SVSHE | (SVSMHRRL0 * (old_level+1));    
    
    // Wait until SVM high-side delay time expires 
    //  (earlier, we disabled an interrupt for this)
    while ((PMMIFG & SVSMHDLYIFG) == 0);  
    
    // Check if a VCore increase is possible
    //  a better option would be to check once, and recover previous settings*
    while ((PMMIFG & SVMHIFG) == SVMHIFG); 
    
    // Set SVM low side to new level
    //  Enable low-side SVM
    //  Set the new Vcore voltage
    SVSMLCTL = SVMLE | (SVSMLRRL0 * (old_level+1));
    
    // Wait until SVM low-side delay time expires 
    while ((PMMIFG & SVSMLDLYIFG) == 0);
    
    // Clear already set flags
    PMMIFG &= ~(SVMLVLRIFG | SVMLIFG);
    
    // Set VCore to a new level
    PMMCTL0_L = PMMCOREV0 * (old_level+1);
    
    // Wait until the new level is reached
    while (PMMIFG & SVMLVLRIFG); 
    
    // clear the delay flag, this time it will be used for low-level side
    PMMIFG &= ~SVSMLDLYIFG;
    
    // enable SVS on the low-level side and 
    //  set the low-side reset voltage level (SVSLRVL)
    SVSMLCTL |= SVSLE | (SVSLRVL0 * (old_level+1));
    
    // wait for lowside delay flags
    while ((PMMIFG & SVSMLDLYIFG) == 0);
    
    // Here you can disable SVS/SVM Low and full-performance mode to save energy
    
    // Clear all Flags
    PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG | SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG);
    // backup PMM-Interrupt-Register
    PMMRIE = PMMRIE_backup;
    
    // Disable write access to PMM registers
    PMMCTL0_H = 0x00;
    
    return 1; // return: OK
  }
  else 
    return 0; // return NOK
}

void toggleLED(void) {
    P1DIR |= 1;
    P1OUT ^= 1;
    /*
    if (P1OUT & 1) {
      P1OUT &= ~1;
    } else {
      P1OUT |= 1;
    }
    */
}


initClock(void) {
  UCSCTL6 &= ~XT1OFF; // TURN ON XTL
  UCSCTL1 |= DCORSEL_5; // SET FREQ RANGE TO 5
  UCSCTL2 = 518; // SET MULTIPLIER TO 518. 518*32kHz = ~17 MHz
  P11SEL |= BIT1 + BIT2; // set pin to mclk and smclk mode
  P11DIR |= BIT1 + BIT2; // set to output
  P7SEL |= BIT0 + BIT1;
  while (UCSCTL7 & XT1LFOFFG){ //CHECK TO SEE IF XTL IS STABLE
    UCSCTL7 &= ~XT1LFOFFG;
  }
}

void sleep(int i) {
  int j;
  for(i=0;i<1000;i++){
    for (j=0;j<1000;j++){
    }
  }
}


#endif
