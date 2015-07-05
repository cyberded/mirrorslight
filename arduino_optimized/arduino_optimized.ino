#define sensorInputPin A0
#define sensorPowerPin 5
#define lightPin 3

#define timeToSwitch 50
#define timeToIntensitySetup 1000

//#define timeToModeSetup 15000

#define switchingLightsSpeed 1000
#define intensitySetupSpeed 7000

//#define isInvertLightsOutput false
#define maxPWMValue 255

unsigned char maxIntencity = maxPWMValue;
unsigned char lastIntencitySetupValue = 0;

boolean lightsState = false;

unsigned int processStarted = 0;

unsigned int timeTrackingStarted = 0;
unsigned int timeTracking = 0;

#define STATE_SWITCHING 1
#define STATE_INTENCITY_SETUP 2
//#define STATE_MODE_SETUP 3

#define NORMAL 1
//#define EXPONENT 2
//#define ARITHMETICAL_ROOT 3
//byte switchingLigtsMode = 1;

byte currentState = 0;
/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 */
//int mem = 0;
void setup() {
  // initialize serial communication at 9600 bits per second:
  /*Serial.begin(9600);
  Serial.print(freeMem);
  Serial.print(delimeter);
  Serial.print(freeRam());*/
  //Serial.begin(9600);
  pinMode(sensorPowerPin, OUTPUT);
  //digitalWrite(sensorPowerPin, HIGH);

}
/*
int freeRam(){
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}*/
// the loop routine runs over and over again forever:
void loop() {
  //Serial.println(freeRam());
  //int timeTracking = 0
  digitalWrite(sensorPowerPin, HIGH);
  delay(1);
  int sensorValue = analogRead(A0);
  //Serial.println(sensorValue);
  digitalWrite(sensorPowerPin, LOW);
  //Serial.print(sensorValue);
  //Serial.print(delimeter);
  if(sensorValue < 500){
      if(timeTrackingStarted == 0){
        timeTrackingStarted = millis();
      }
      timeTracking = millis() - timeTrackingStarted;
  } else {
    if(sensorValue > 600){
      timeTrackingStarted = 0;
      timeTracking = 0;
    } 
  }
  //Проверяем нужно ли включить или выключить
  /*if(timeTracking == 0 && timeToSwitch < lastTimeTracking && lastTimeTracking < timeToIntensitySetup){
      triggerSwitchLight(time);
  }*/
  if(timeTracking > 0 && (currentState == 0) ){
    processStarted = millis();
    if(lightsState){
        lightsState = false;
    } else {
        lightsState = true;
    }
    currentState = STATE_SWITCHING; 
  }
  //Проверяем завершили ли настройку яркости
  if(timeTracking == 0 && currentState == STATE_INTENCITY_SETUP){
     currentState = 0;
     //if(lastIntencitySetupValue != 0){
       maxIntencity = lastIntencitySetupValue;
     //}
     lightsState = true;
     timeTrackingStarted = 0;    
  }
  //Проверяем начинать ли настройку яркости
  if( timeToIntensitySetup < timeTracking && timeTracking < (timeToIntensitySetup + intensitySetupSpeed) && currentState != STATE_INTENCITY_SETUP){
        processStarted = millis();
        currentState = STATE_INTENCITY_SETUP; 
  }
  if(currentState == STATE_INTENCITY_SETUP){

     unsigned int value;// = 0;
     
     //calculate value based on the time left since starting switching on
      //Добавляем секунду ожидания перед стартом
      if(millis() - processStarted < 1000){
          value = 0;
      } else {
        /*value = (millis() - 1000) - processStarted;
        value = _map(value, 0, intensitySetupSpeed, 0, maxPWMValue);*/
        //value = maxPWMValue - (millis - 1000) - processStarted;
          value = _map(millis() + 1000 - processStarted, 0, intensitySetupSpeed, 0, maxPWMValue);
          //value = value - maxPWMValue;
        //Если выключаем, то инвертируем значение
      }
     
     lastIntencitySetupValue = value;
      /*if(isInvertLightsOutput){
         value = value - maxPWMValue;
      }*/
       
      analogWrite(lightPin, value);
      
      if(millis() > processStarted + intensitySetupSpeed + 1000){
          
          maxIntencity = maxPWMValue;
          analogWrite(lightPin, maxPWMValue);
          
          //currentState = 0;
      }
  }
  if(currentState == STATE_SWITCHING){
     //calculate value based on the time left since starting switching on
      unsigned int value = millis() - processStarted;
      value = _map(value, 0, switchingLightsSpeed, 0, maxIntencity);
      //Если выключаем, то инвертируем значение
      if(!lightsState){
        value = maxIntencity - value;
      }
     
      //value = calculateNormalLight(lightsState);
     /* if(isInvertLightsOutput){
         value = maxIntencity - value;
      }*/
      analogWrite(lightPin, value);
      
      if(millis() > processStarted + switchingLightsSpeed){

          currentState = 0;
           //millis = 0;
      }
  }
  
 //Serial.println(freeRam());
  //Serial.println(mem);
  //processIntencitySetup();
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
