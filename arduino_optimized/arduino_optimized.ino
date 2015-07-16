#include <EEPROM.h>

#define sensorInputPin 6
#define sensorPowerPin 5
#define lightPin 3

#define timeToSwitch 50
#define timeToIntensitySetup 1000

#define sensorMeasuresCount 5
//#define timeToModeSetup 15000

#define switchingLightsSpeed 1000
#define intensitySetupSpeed 7000

//#define isInvertLightsOutput false
#define maxPWMValue 255

#define EEPROMAddr 1

unsigned char maxIntencity = maxPWMValue;
unsigned char lastMaxIntencity = maxPWMValue;
unsigned char lastIntencitySetupValue = 0;

boolean lightsState = false;

unsigned int processStarted = 0;

unsigned int timeTrackingStarted = 0;
unsigned int timeTracking = 0;

int sensorCounter = 0;
boolean lastSensorValue = false;

#define STATE_SWITCHING 1
#define STATE_INTENCITY_SETUP 2
//#define STATE_MODE_SETUP 3

#define NORMAL 1

byte currentState = 0;



unsigned int _millis = 0;
unsigned long prevMillis = 0;

int debugCounter = 0;
/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

 This example code is in the public domain.
 */
//int mem = 0;
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  //Serial.begin(9600);
  pinMode(sensorInputPin, INPUT);
  pinMode(sensorPowerPin, OUTPUT);
  digitalWrite(sensorPowerPin, HIGH);
  analogWrite(lightPin, 0);
  prevMillis = millis();
  unsigned char tempMaxIntencity = EEPROM.read(EEPROMAddr);
  if (tempMaxIntencity != 0){
    maxIntencity = tempMaxIntencity;
  }
}

void processEmulMillis() {
  unsigned long temp = (millis() - prevMillis);
  _millis = _millis + temp;
  prevMillis = millis();
}
/*
int freeRam(){
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}*/

//Функция пропорционально переносит значение (value) из текущего диапазона значений (fromLow .. fromHigh) в новый диапазон (toLow .. toHigh), заданный параметрами.
unsigned int _map(unsigned int x, unsigned int in_min, unsigned int in_max, unsigned int out_min, unsigned int out_max)
{
  unsigned int result;//unsigned int result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  unsigned long temp = (x - in_min);
  temp = temp * (out_max - out_min);
  result = temp / (in_max - in_min) + out_min;
  //Serial.println(freeRam());


  if (result > out_max) {
    return out_max;
  } else {
    return result;
  }
}

boolean getAndFilterSensorValue(){
  boolean result = lastSensorValue;
  //Изменяем значение только если много раз подряд появилось новое
  boolean tempSensorValue = digitalRead(sensorInputPin);
 /* Serial.print("tempSensorValue ");
  Serial.println(tempSensorValue);*/
  if(tempSensorValue){
     debugCounter++;
  }
  if (tempSensorValue) {
    if(sensorCounter < -3 ){
        sensorCounter = sensorCounter + 3;
    } else {
      if (sensorCounter > sensorMeasuresCount){
        result = true;
      } else {
        sensorCounter++;
      }
    }
  } else {
      if (sensorCounter < (-1 * sensorMeasuresCount)){
        result = false;
      } else {
        sensorCounter--;
      }

  }
  
  lastSensorValue = result;
  return result;
}

// the loop routine runs over and over again forever:
void loop() {
  unsigned long tempTime = micros();
  processEmulMillis();
  //Serial.println(freeRam());
  //int timeTracking = 0
  //digitalWrite(sensorPowerPin, HIGH);
  //delay(1);
  
  //Serial.println(sensorValue);

  //digitalWrite(sensorPowerPin, LOW);
  //Serial.print(sensorValue);
  //Serial.print(delimeter);
  boolean sensorValue = getAndFilterSensorValue();
  //  Serial.print("sensorValue ");
  //Serial.println(sensorValue);
  if (sensorValue) {
      if (timeTrackingStarted == 0) {
        timeTrackingStarted = _millis;
      }
      timeTracking = _millis - timeTrackingStarted;
  } else {
      timeTrackingStarted = 0;
      timeTracking = 0;
  }
  //Проверяем нужно ли включить или выключить
  /*if(timeTracking == 0 && timeToSwitch < lastTimeTracking && lastTimeTracking < timeToIntensitySetup){
      triggerSwitchLight(time);
  }*/
  if ((timeTracking > 0) && (currentState == 0) ) {
    processStarted = _millis;
    if (lightsState) {
      lightsState = false;
    } else {
      lightsState = true;
    }
    currentState = STATE_SWITCHING;
  }
  //Проверяем завершили ли настройку яркости
  if (timeTracking == 0 && currentState == STATE_INTENCITY_SETUP) {
    currentState = 0;
    if(lastIntencitySetupValue != 0){
      maxIntencity = lastIntencitySetupValue;
      EEPROM.write(EEPROMAddr, maxIntencity);
    } else {
      analogWrite(lightPin, maxIntencity);
    } 
  lightsState = 1;
  timeTrackingStarted = 0;
  _millis = 0;
}
  //Проверяем начинать ли настройку яркости
  if ( timeToIntensitySetup < timeTracking && timeTracking < (timeToIntensitySetup + intensitySetupSpeed) && currentState != STATE_INTENCITY_SETUP) {
    processStarted = _millis;
    currentState = STATE_INTENCITY_SETUP;
  }
  
  if(currentState == STATE_INTENCITY_SETUP){

      unsigned int value;// = maxPWMValue;
     
    //calculate value based on the time left since starting switching on
    //Добавляем секунду ожидания перед стартом
    if (_millis - processStarted < 1000) {
      value = 0;
    } else {

      //value = maxPWMValue - (millis - 1000) - processStarted;
      value = _map(_millis - 1000 - processStarted, 0, intensitySetupSpeed, 0, maxPWMValue);
      //value = value - maxPWMValue;
      //Если выключаем, то инвертируем значение
    }

    lastIntencitySetupValue = value;
    /*if(isInvertLightsOutput){
       value = value - maxPWMValue;
    }*/
    analogWrite(lightPin, value);
    if (_millis > processStarted + intensitySetupSpeed + 1000) {
      if(maxIntencity != maxPWMValue){
        maxIntencity = maxPWMValue;
        EEPROM.write(EEPROMAddr, maxIntencity);
      }
      analogWrite(lightPin, maxPWMValue);
      //currentState = 0;
    }
    if (_millis > processStarted + intensitySetupSpeed + 1000 + 20000) {
      timeTrackingStarted = 0;
      timeTracking = 0;
      processStarted = 0;
      currentState = 0;
      lightsState = 0;
      _millis = 0;
    }
  }
  if (currentState == STATE_SWITCHING) {
    //calculate value based on the time left since starting switching on
    unsigned int value = _millis - processStarted;
    value = _map(value, 0, switchingLightsSpeed, 0, maxIntencity);
    //Если выключаем, то инвертируем значение
    if (!lightsState) {
      value = maxIntencity - value;
    }

    //value = calculateNormalLight(lightsState);
    analogWrite(lightPin, value);

    if (_millis > (processStarted + switchingLightsSpeed) && timeTracking == 0) {
      currentState = 0;
      //processStarted = 0;
      _millis = 0;
    }
  }
  
  /*Serial.print("tempTime = ");
  Serial.println(micros()- tempTime);*/
  //processIntencitySetup();
}




