#include <tiny13a.h>
#include <delay.h>

#define sensorInputPin PINB.3
#define sensorPowerPin PORTB.4
#define lightPin1 OCR0A
#define lightPin2 OCR0B

#define timeToIntensitySetup 1000

//#define timeToModeSetup 15000

#define switchingLightsSpeed 1000
#define intensitySetupSpeed 7000

#define maxPWMValue 255

unsigned char maxIntencity = maxPWMValue;
unsigned char lastIntencitySetupValue = 0;

//bit processingIntencitySetup = 0;
bit lightsState = 0;

unsigned int processStarted = 0;

unsigned int millis = 0;
unsigned int timeTrackingStarted = 0;
unsigned int timeTracking = 0;

#define STATE_SWITCHING 1
#define STATE_INTENCITY_SETUP 2

#define NORMAL 1
//char switchingLigtsMode = 1;

char currentState = 0;


interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
TCNT0=0x69;
millis++;
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
//Функция пропорционально переносит значение (value) из текущего диапазона значений (fromLow .. fromHigh) в новый диапазон (toLow .. toHigh), заданный параметрами.
unsigned int _map(unsigned int x, unsigned int in_min, unsigned int in_max, unsigned int out_min, unsigned int out_max)
{
  //unsigned int result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  unsigned long temp = (x - in_min);
  temp = temp * (out_max - out_min);
  unsigned int result = temp / (in_max - in_min) + out_min;
  //Serial.println(freeRam());
  
  
  if(result > out_max){
     return out_max;
  } else {
    return result;
  }
}

void analogWrite(unsigned char temp_value)
{
lightPin1 = maxPWMValue - temp_value;
lightPin2 = maxPWMValue - temp_value;
}

void main(void)
{
init_avr();

while (1)
{
unsigned char sensorValue;
sensorPowerPin = 1;
delay_us(1000);

ADCSRA|=0x40;
while ((ADCSRA & 0x10)==0);
ADCSRA|=0x10;
sensorValue = ADCW>>2;

sensorPowerPin = 0;
      
      if(sensorValue < 125){
        if(timeTrackingStarted == 0){
            timeTrackingStarted = millis;
        }
        timeTracking = millis - timeTrackingStarted;
      } else {
        if(sensorValue > 160){
            timeTrackingStarted = 0;
            timeTracking = 0;
        } 
      }
      //Проверяем нужно ли включить или выключить
      if((timeTracking > 0) && (currentState == 0)){
        processStarted = millis;
        if(lightsState){
            lightsState = 0;
        } else {
            lightsState = 1;
        }
        currentState = STATE_SWITCHING; 
      }
      //Проверяем завершили ли настройку яркости
      if(timeTracking == 0 && currentState == STATE_INTENCITY_SETUP){
            currentState = 0;
	     if(lastIntencitySetupValue != 0){
	         maxIntencity = lastIntencitySetupValue;
	     } else {
	         analogWrite(lightPin, maxIntencity);
	     }
            lightsState = 1;
            timeTrackingStarted = 0;    
	    millis = 0;
        }
  //Проверяем начинать ли настройку яркости
  if( timeToIntensitySetup < timeTracking && timeTracking < (timeToIntensitySetup + intensitySetupSpeed) && currentState != STATE_INTENCITY_SETUP){
        processStarted = millis;
        currentState = STATE_INTENCITY_SETUP; 
  }
  
  if(currentState == STATE_INTENCITY_SETUP){

      unsigned int value;// = maxPWMValue;
     
     //calculate value based on the time left since starting switching on
      //Добавляем секунду ожидания перед стартом
      if(millis - processStarted < 1000){
          value = 0;
      } else {
          //value = maxPWMValue - (millis - 1000) - processStarted;
          value = _map(millis - processStarted, 0, intensitySetupSpeed, 0, maxPWMValue);
          //value = value - maxPWMValue;
        //Если выключаем, то инвертируем значение
      }
     
     lastIntencitySetupValue = value;
     /*if (isInvertLightsOutput){
         value = value - maxPWMValue;
     }*/
      analogWrite(value);
      
      if(millis > processStarted + intensitySetupSpeed + 1000){
	  maxIntencity = maxPWMValue;
          analogWrite(lightPin, maxPWMValue);          
          //currentState = 0;
      }
      if(millis() > processStarted + intensitySetupSpeed + 1000 + 20000){
        timeTrackingStarted = 0;
        timeTracking = 0;
        processStarted = 0;
        currentState = 0;
        lightsState = false;
        millis = 0;
      }
  }
  if(currentState == STATE_SWITCHING){
     //calculate value based on the time left since starting switching on
      unsigned int value = millis - processStarted;
      value = _map(value, 0, switchingLightsSpeed, 0, maxIntencity);
      //Если выключаем, то инвертируем значение
      if(!lightsState){
        value = maxIntencity - value;
      }
     
      //value = calculateNormalLight(lightsState);
      analogWrite(value);
      
      if(millis() > processStarted + switchingLightsSpeed){
 	   currentState = 0;
           processStarted = 0;
	   millis = 0;
      }

  }
 

}
}

void reset(){
      timeTrackingStarted = 0;
      timeTracking = 0;
      processStarted = 0;
      currentState = 0;
      lightsState = false;
      //millis = 0;
}


