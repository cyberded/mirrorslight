
#include <tiny13a.h>
#include <delay.h>
#define PWM1            OCR0A
#define PWM2            OCR0B
#define DAT             PINB.3
#define LED             PORTB.4


unsigned char mode = 0, t = 0, power = 0;

void init_avr(void)
{
// Declare your local variables here

// Crystal Oscillator division factor: 1
#pragma optsize-
CLKPR=0x80;
CLKPR=0x00;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Input/Output Ports initialization
// Port B initialization
// Func5=In Func4=Out Func3=In Func2=In Func1=Out Func0=Out 
// State5=T State4=0 State3=T State2=T State1=0 State0=0 
PORTB=0x00;
DDRB=0x13;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 1200,000 kHz
// Mode: Fast PWM top=0xFF
// OC0A output: Non-Inverted PWM
// OC0B output: Non-Inverted PWM
TCCR0A=0xf3;
TCCR0B=0x02;
TCNT0=0x00;
OCR0A=0x00;
OCR0B=0x00;

// External Interrupt(s) initialization
// INT0: Off
// Interrupt on any change on pins PCINT0-5: Off
GIMSK=0x00;
MCUCR=0x00;

// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=0x00;

// Analog Comparator initialization
// Analog Comparator: Off
ACSR=0x80;
ADCSRB=0x00;
DIDR0=0x00;

// ADC initialization
// ADC disabled
ADCSRA=0x00;
}

void main(void)
{
init_avr();
PWM1 = 255;
PWM2 = 0;
while (1)
      { 
      LED = 1;
      delay_us(10);
      if(DAT&&t>0) t--;
      if(!DAT&&t<255) t++; 
      LED = 0;
      
      switch (mode)
        {
        case 0:
        if (t>10) mode = 1;
        break;
        case 1:
        if (t==0) mode = 2;
        //if (t==255) mode = 3;
        break; 
        case 2:
        if (PWM1 == power) PWM1 = 255;
        else PWM1 = power;
        mode = 0;
        break;
        
        }
        
        
        
        
      }
}
