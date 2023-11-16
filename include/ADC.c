#include <msp430.h>


float slope;
float inter0;
unsigned int temp30;
unsigned int temp85;
float temp;
int tempFlag=0;

void initTemp(void){
REFCTL0 |= REFON + REFMSTR + REFOUT; // TURN ON REF VOLTAGE FOR 1.5 V, AND SET TO OUTPUT
ADC12CTL0 |= ADC12ON + ADC12SHT0_12; //turn on ADC and sample for 1024 cycles
ADC12CTL1 = ADC12SSEL_3 + ADC12CONSEQ_0 + ADC12SHP + ADC12DIV_1;// USE SMCLK FOR ADC12, SET TO SINGLE CHANNEL, divide clock by 2
ADC12IE = ADC12IE0;
ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10 + ADC12EOS; // END SEQUENCE ON 0

temp30 = *((unsigned int*)0x01A1A);//get TVL vals for temp conversion
temp85 = *((unsigned int*)0x01A1C);
inter0 = temp30;
slope = (temp85-inter0)/(85-30);

ADC12CTL0 |= ADC12ENC; // ENABLE CONVERSION
}

void getTemp(void){
ADC12CTL0 |= ADC12SC; // START CONVERSION
while(tempFlag==0){
LPM0;
}
tempFlag=0;
}


void tempDone(void) __interrupt [ADC12_VECTOR]{
temp = ADC12MEM0;
temp = (temp - temp30);
temp = temp/slope;
temp += 30;
tempFlag=1;
LPM0_EXIT;
}
