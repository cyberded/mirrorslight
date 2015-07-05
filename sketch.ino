#define sensorInputPin A0
#define sensorPowerPin 5
#define lightPin 3

#define isDebugEnabled true

#define timeToSwitch 50
#define timeToIntensitySetup 2000

#define timeToModeSetup 15000

#define switchingLightsSpeed 2000
#define intensitySetupSpeed 5000
#define modeSetupSpeed 1000

#define isInvertLightsOutput false
#define maxPWMValue 255

int maxIntencity = maxPWMValue;

unsigned long lastTimeTracking = 0;
boolean processingIntencitySetup = false;
boolean lightsState = false;

unsigned long switchingLightsStarted = 0;
unsigned long intencitySetupStarted = 0;
unsigned long modeSetupStarted = 0;

unsigned long timeTrackingStarted = 0;

#define STATE_SWITCHING 1
#define STATE_INTENCITY_SETUP 2
#define STATE_MODE_SETUP 3

#define NORMAL 1
#define EXPONENT 2
#define ARITHMETICAL_ROOT 3
byte switchingLigtsMode = 1;

byte currentState = 0;
/*
AnalogReadSerial
Reads an analog input on pin 0, prints the result to the serial monitor.
Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

This example code is in the public domain.
*/

// the setup routine runs once when you press reset:
void setup() {
	// initialize serial communication at 9600 bits per second:
	Serial.begin(9600);
	initPins();
}

void initPins(){
	pinMode(sensorPowerPin, OUTPUT);
	digitalWrite(sensorPowerPin, HIGH);

	pinMode(lightPin, OUTPUT);
	//digitalWrite(lightPin, HIGH);

	if (isInvertLightsOutput){
		analogWrite(lightPin, maxPWMValue);
	}
	else{
		analogWrite(lightPin, 0);
	}
}

// the loop routine runs over and over again forever:
void loop() {
	unsigned long time = millis();
	unsigned long timeTracking = getTimeTracking();
	if (isDebugEnabled){
		Serial.print("getTimeTracking returned ");
		Serial.println(getTimeTracking());
	}
	//Проверяем нужно ли включить или выключить
	/*if(timeTracking == 0 && timeToSwitch < lastTimeTracking && lastTimeTracking < timeToIntensitySetup){
	triggerSwitchLight(time);
	}*/
	if (timeTracking > 0 && currentState != STATE_SWITCHING){
		triggerSwitchLight(time);
	}
	//Проверяем завершили ли настройку яркости
	if (timeTracking == 0 && currentState == STATE_INTENCITY_SETUP){
		currentState = 0;
	}
	//Проверяем начинать ли настройку яркости
	if (timeToIntensitySetup < timeTracking && timeTracking < (timeToIntensitySetup + intensitySetupSpeed) && currentState != STATE_INTENCITY_SETUP){
		triggerIntencitySetup(time);
	}
	//Проверяем начинать ли настройку режимов
	if (timeToModeSetup < timeTracking && currentState != STATE_MODE_SETUP){
		triggerModeSetup(time);
	}


	processSwitchLight();
	processIntencitySetup();
	processModeSetup();

	lastTimeTracking = timeTracking;
	if (isDebugEnabled){
		delay(200);
	}
}

//Returns the time 
unsigned long getTimeTracking(){
	int sensorValue = analogRead(A0);
	if (sensorValue < 800){
		if (timeTrackingStarted == 0){
			timeTrackingStarted = millis();
		}
		return millis() - timeTrackingStarted;
	}
	else {
		timeTrackingStarted = 0;
		return 0;
	}
}

void triggerSwitchLight(unsigned long time){
	if (isDebugEnabled){
		Serial.println("Switching light triggered. ");
	}
	switchingLightsStarted = time;
	if (lightsState){
		lightsState = false;
	}
	else {
		lightsState = true;
	}
	if (isDebugEnabled){
		Serial.print("LightsState = ");
		Serial.println(lightsState);
	}
	currentState = STATE_SWITCHING;
}
void triggerIntencitySetup(unsigned long time){
	intencitySetupStarted = time;
	if (isDebugEnabled){
		Serial.println("Intencity setup triggered. ");
	}
	currentState = STATE_INTENCITY_SETUP;
}
void triggerModeSetup(unsigned long time){
	modeSetupStarted = time;
	currentState = STATE_MODE_SETUP;
}

void processSwitchLight(){
	if (currentState == STATE_SWITCHING){
		int value = 0;
		switch (switchingLigtsMode){
		case NORMAL: value = calculateNormalLight(lightsState); break;
		case EXPONENT: value = calculateExponentLight(lightsState); break;
		case ARITHMETICAL_ROOT: value = calculateArithmeticalRootLight(lightsState); break;
		default:
			Serial.print("Error! No switch ligts mode with number ");
			Serial.print(switchingLigtsMode);
			Serial.println(" found.");
		}
		if (isInvertLightsOutput){
			value = maxIntencity - value;
		}
		analogWrite(lightPin, value);

		if (millis() > switchingLightsStarted + switchingLightsSpeed){

			currentState = 0;
		}
	}

}
void processIntencitySetup(){
	if (currentState == STATE_INTENCITY_SETUP){

		int value = calculateIntencitySetupLight();
		maxIntencity = value;
		if (isInvertLightsOutput){
			value = value - maxPWMValue;
		}

		analogWrite(lightPin, value);

		if (millis() > intencitySetupStarted + intensitySetupSpeed){
			maxIntencity = maxPWMValue;
			analogWrite(lightPin, maxPWMValue);
			currentState = 0;
		}
	}

}
void processModeSetup(){
	if (currentState == STATE_MODE_SETUP){
		int value = calculateModeSetupLight();
		if (isInvertLightsOutput){
			value = value - maxPWMValue;
		}
		analogWrite(lightPin, value);

		switch (switchingLigtsMode){
		case NORMAL: switchingLigtsMode = EXPONENT; break;
		case EXPONENT: switchingLigtsMode = ARITHMETICAL_ROOT; break;
		case ARITHMETICAL_ROOT: switchingLigtsMode = NORMAL; break;
		default:
			Serial.print("Error! No switch ligts mode with number ");
			Serial.print(switchingLigtsMode);
			Serial.println(" found.");
		}

		if (millis() > modeSetupStarted + modeSetupSpeed){
			lightsState = false;
			if (isInvertLightsOutput){
				analogWrite(lightPin, maxPWMValue);
			}
			else{
				analogWrite(lightPin, 0);
			}
			currentState = 0;
		}
	}

}
int calculateNormalLight(boolean lightsState){

	//calculate value based on the time left since starting switching on
	unsigned long time = millis();
	int value = _map(time - switchingLightsStarted, 0, switchingLightsSpeed, 0, maxIntencity);
	//Если выключаем, то инвертируем значение
	if (!lightsState){
		value = maxIntencity - value;
		if (isDebugEnabled){
			Serial.println("Inverted value.");
		}
	}
	return value;

}

int calculateExponentLight(boolean lightsState){
	return calculateNormalLight(lightsState);

}
int calculateArithmeticalRootLight(boolean lightsState){

	return calculateNormalLight(lightsState);

}

int calculateIntencitySetupLight(){

	//calculate value based on the time left since starting switching on
	unsigned long time = millis();
	return _map(time - intencitySetupStarted, 0, intensitySetupSpeed, 0, maxPWMValue);
	//Если выключаем, то инвертируем значение

}
//Делаем вспышку длинной в ону секунду
int calculateModeSetupLight(){
	unsigned long time = millis();
	int value = _map(time - modeSetupStarted, 0, modeSetupSpeed / 2, 0, maxIntencity);

	if ((millis() - modeSetupStarted) > modeSetupSpeed / 2){
		value = maxIntencity - value;
	}
	//calculate value based on the time left since starting switching on
	return value;


}
//Функция пропорционально переносит значение (value) из текущего диапазона значений (fromLow .. fromHigh) в новый диапазон (toLow .. toHigh), заданный параметрами.
unsigned long _map(unsigned long x, unsigned long in_min, unsigned long in_max, unsigned long out_min, unsigned long out_max)
{
	unsigned long result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	if (isDebugEnabled){
		Serial.print("Map params is ");
		Serial.print("x=");
		Serial.print(x);
		Serial.print(" in_min=");
		Serial.print(in_min);
		Serial.print(" in_max=");
		Serial.print(in_max);
		Serial.print(" out_min=");
		Serial.print(out_min);
		Serial.print(" out_max=");
		Serial.print(out_max);
		Serial.print(". Returns ");
		Serial.println(result);
	}
	if (result > out_max){
		return out_max;
	}
	else {
		return result;
	}
}