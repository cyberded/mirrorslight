#include <tiny13a.h>
#include <delay.h>

#define sensorInputPin PINB.3
#define sensorPowerPin PORTB.4
#define lightPin1 OCR0A
#define lightPin2 OCR0B

#define timeToSwitch 50
#define timeToIntensitySetup 1000

#define timeToModeSetup 15000

#define switchingLightsSpeed 1000
#define intensitySetupSpeed 7000

#define isInvertLightsOutput 0
#define maxPWMValue 255

int maxIntencity = maxPWMValue;
int lastIntencitySetupValue = 0;

bit processingIntencitySetup = 0;
bit lightsState = 0;

unsigned int processStarted = 0;

unsigned int millis = 0;
unsigned int timeTrackingStarted = 0;
unsigned int timeTracking = 0;

#define STATE_SWITCHING 1
#define STATE_INTENCITY_SETUP 2
#define STATE_MODE_SETUP 3

#define NORMAL 1
#define EXPONENT 2
#define ARITHMETICAL_ROOT 3
char switchingLigtsMode = 1;

char currentState = 0;


interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
TCNT0=0x69;
millis++;

}

unsigned int analogRead(void)
{
ADMUX=0x03 | 0x00;
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=0x40;
// Wait for the AD conversion to complete
while ((ADCSRA & 0x10)==0);
ADCSRA|=0x10;
return ADCW;
}

void init_avr (void)
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
// Clock value: 150,000 kHz
// Mode: Fast PWM top=0xFF
// OC0A output: Inverted PWM
// OC0B output: Inverted PWM
TCCR0A=0xF3;
TCCR0B=0x03;
TCNT0=0x69;
OCR0A=0x00;
OCR0B=0x00;

// External Interrupt(s) initialization
// INT0: Off
// Interrupt on any change on pins PCINT0-5: Off
GIMSK=0x00;
MCUCR=0x00;

// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=0x02;

// Analog Comparator initialization
// Analog Comparator: Off
ACSR=0x80;
ADCSRB=0x00;
DIDR0=0x00;

// ADC initialization
// ADC Clock frequency: 75,000 kHz
// ADC Bandgap Voltage Reference: Off
// ADC Auto Trigger Source: Free Running
// Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: Off
DIDR0&=0x03;
DIDR0|=0x08;
ADMUX=0x03;
ADCSRA=0xA7;
ADCSRB&=0xF8;

// Global enable interrupts
#asm("sei")
}

//������� ��������������� ��������� �������� (value) �� �������� ��������� �������� (fromLow .. fromHigh) � ����� �������� (toLow .. toHigh), �������� �����������.
unsigned long _map(unsigned long x, unsigned long in_min, unsigned long in_max, unsigned long out_min, unsigned long out_max)
{
  unsigned long result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  //Serial.println(freeRam());
  
  
  if(result > out_max){
     return out_max;
  } else {
    return result;
  }
  
      }

void analogWrite(unsigned char temp_value)
{
lightPin1 = temp_value;
lightPin2 = temp_value;
}

void main(void)
{
init_avr();

if(isInvertLightsOutput){
    analogWrite(maxPWMValue);
  } else{
    analogWrite(0);
  }


while (1)
{
int sensorValue;
sensorPowerPin = 1;
delay_us(1000);
sensorValue = analogRead();
sensorPowerPin = 0;
      
      if(sensorValue < 500){
        if(timeTrackingStarted == 0){
            timeTrackingStarted = millis;
        }
        timeTracking = millis - timeTrackingStarted;
      } else {
        if(sensorValue > 600){
            timeTrackingStarted = 0;
            timeTracking = 0;
        } 
      }
      //��������� ����� �� �������� ��� ���������
      if((timeTracking > 0) && (currentState == 0)){
        processStarted = millis;
        if(lightsState){
            lightsState = 0;
        } else {
            lightsState = 1;
        }
        currentState = STATE_SWITCHING; 
      }
      //��������� ��������� �� ��������� �������
      if(timeTracking == 0 && currentState == STATE_INTENCITY_SETUP){
     currentState = 0;
     if(lastIntencitySetupValue != 0){
       maxIntencity = lastIntencitySetupValue;
     }
     lightsState = 1;
  }
  //��������� �������� �� ��������� �������
  if( timeToIntensitySetup < timeTracking && timeTracking < (timeToIntensitySetup + intensitySetupSpeed) && currentState != STATE_INTENCITY_SETUP){
        processStarted = millis;
        currentState = STATE_INTENCITY_SETUP; 
  }
  
  if(currentState == STATE_SWITCHING){
     //calculate value based on the time left since starting switching on
      int value = millis - processStarted;
      value = _map(value, 0, switchingLightsSpeed, 0, maxIntencity);
      //���� ���������, �� ����������� ��������
      if(!lightsState){
        value = maxIntencity - value;
      }
     
      //value = calculateNormalLight(lightsState);
      if(isInvertLightsOutput){
         value = maxIntencity - value;
      }
      analogWrite(value);
      
      if(millis > processStarted + switchingLightsSpeed){

          currentState = 0;
      }
  }
  if(currentState == STATE_INTENCITY_SETUP){

     int value = 0;
     
     //calculate value based on the time left since starting switching on
      //��������� ������� �������� ����� �������
      if(millis - processStarted < 1000){
          value = 0;
      } else {
        value = (millis - 1000) - processStarted;
        value = _map(value, 0, intensitySetupSpeed, 0, maxPWMValue);
        //���� ���������, �� ����������� ��������
      }
     
     lastIntencitySetupValue = value;
      if(isInvertLightsOutput){
         value = value - maxPWMValue;
      }
       
      analogWrite(value);
      
      if(millis > processStarted + intensitySetupSpeed + 1000){
          
          maxIntencity = maxPWMValue;
          analogWrite(maxPWMValue);
          
          currentState = 0;
      }
  }

}
}


