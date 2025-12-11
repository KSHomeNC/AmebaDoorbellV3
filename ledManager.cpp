#include <Arduino.h>
#include "ledManager.h"
#include <GTimer.h>

#define TIMER_ID 1


LEDManager *ledMgr=NULL;
void timer_handler_led(uint32_t data)
{ if(ledMgr != NULL){  
    ledMgr->loop();
  }
}

LEDManager::LEDManager() {
      
  currentLedPatternState = LED_PATTERN_UNDEF;
  PreviousLedPatternState = LED_PATTERN_UNDEF;
  multipleLedToggleDuration = 0; 
  multipleLedToggleStartT = 0; 
  multipleLedCnt = 0;
}

void LEDManager:: begin() {
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  ledMgr = this;

  ledArray[BLUE].ledPin = BLUE_LED_PIN;
  ledArray[BLUE].led.onDuration = 0;
  ledArray[BLUE].led.offDuration = 0;
  ledArray[BLUE].led.defaultState = false; // off
 
  ledArray[GREEN].ledPin = GREEN_LED_PIN;
  ledArray[GREEN].led.onDuration = 0;
  ledArray[GREEN].led.offDuration = 0;
  ledArray[GREEN].led.defaultState = false; // off
  
  ledArray[RED].ledPin = RED_LED_PIN;
  ledArray[RED].led.onDuration = 0;
  ledArray[RED].led.offDuration = 0;
  ledArray[RED].led.defaultState = false; // off
  
  PreviousLedPatternState = LED_PATTERN_NONE;
  currentLedPatternState = LED_PATTERN_NONE;
  GTimer.begin(TIMER_ID, ( 100 * 1000), timer_handler_led); // first for 2 Sec at boot up time
}



void LEDManager::setNextPatternNone( LEDColor led) {
  
  PreviousLedPatternState = LED_PATTERN_NONE;
  currentLedPatternState = LED_PATTERN_NONE;
  
}

void LEDManager::setNextPatternSingle( LEDColor led, LEDinfo* ledinfo) {
  
  if( (led >= BLUE) && (led < LED_END)){    
    ledArray[led].led.onDuration = ledinfo->onDuration;
    ledArray[led].led.offDuration = ledinfo->offDuration;
    ledArray[led].led.defaultState = ledinfo->defaultState;
    ledArray[led].ledState =ledinfo->defaultState;
    ledArray[led].startTime = 0;
    PreviousLedPatternState = LED_PATTERN_NONE;
    currentLedPatternState = LED_PATTERN_SIGNLE;
  }
  
}
void LEDManager::setNextPatternMultiple(unsigned long duration) {
  multipleLedToggleDuration = duration;  
  multipleLedToggleStartT = millis();
  PreviousLedPatternState = currentLedPatternState;
  multipleLedCnt=0;
  multipleLedloopStartT = 0;
  onTime=false;
  currentLedPatternState = LED_PATTERN_MULTIPLE;
}
bool LEDManager::isLedTimerExpire(unsigned long startT, unsigned long duration ){
  return ( (millis()-startT)> duration ? true : false);
}
void LEDManager::singlePatternHandler() {
  for(int i=0; i<MAX_LEDS; i++){
  
    if (((ledArray[i].led.onDuration == 0 && ledArray[i].led.offDuration == 0)) ||
      ((ledArray[i].led.onDuration == 0 && ledArray[i].led.offDuration != 0)) ){// always off
      digitalWrite(ledArray[i].ledPin, LOW);
    }else if(ledArray[i].led.onDuration != 0 && ledArray[i].led.offDuration == 0){ // always ON
      digitalWrite(ledArray[i].ledPin, HIGH);
    }else if(ledArray[i].led.onDuration != 0 && ledArray[i].led.offDuration != 0){ // blinking
      if(ledArray[i].startTime == 0){
        ledArray[i].startTime = millis();
        ledArray[i].ledState  = ledArray[i].led.defaultState;
        digitalWrite(ledArray[i].ledPin, ledArray[i].ledState);
        if(ledArray[i].ledState == true){ // on time
          ledArray[i].duration = ledArray[i].led.onDuration;
        }else{
          ledArray[i].duration = ledArray[i].led.offDuration;
        }
      }else{
        if(isLedTimerExpire(ledArray[i].startTime, ledArray[i].duration)){
          ledArray[i].ledState = !ledArray[i].ledState; // toggle the led status
          if(ledArray[i].ledState == true){ // on time
          ledArray[i].duration = ledArray[i].led.onDuration;
          }else{
            ledArray[i].duration = ledArray[i].led.offDuration;
          }
          digitalWrite(ledArray[i].ledPin, ledArray[i].ledState);
          ledArray[i].startTime = millis();      
        }
        
      }
    }
  }
}

void LEDManager::multiplePatternHandler() { // making 1 sec pattern
  if(isLedTimerExpire(multipleLedToggleStartT, multipleLedToggleDuration)){
    currentLedPatternState = PreviousLedPatternState;
  }else{
    if(multipleLedloopStartT == 0){
      multipleLedloopStartT=millis();
      digitalWrite(ledArray[multipleLedCnt++].ledPin,true);
      onTime= true;
    }else{
      if(onTime && isLedTimerExpire(multipleLedloopStartT, 1000)){ // one sec
        digitalWrite(ledArray[multipleLedCnt++].ledPin,true);
        multipleLedloopStartT=millis();
        if(multipleLedCnt> MAX_LEDS){
          multipleLedCnt = 0;
          onTime = false;
        }
        
      }else if(isLedTimerExpire(multipleLedloopStartT, 1000)){
        digitalWrite(ledArray[multipleLedCnt++].ledPin,false);
        multipleLedloopStartT=millis();
        if(multipleLedCnt> MAX_LEDS){
          multipleLedCnt = 0;
          onTime = true;
        }
      }
    }

  }         
}
void LEDManager:: nonePatternHandler(){
   for(int i=0; i<MAX_LEDS; i++){
      digitalWrite(ledArray[i].ledPin, LOW);
  }
}
void LEDManager::loop() {
  
  switch(currentLedPatternState)
  {
    case LED_PATTERN_NONE:
      nonePatternHandler();
      break;
    case LED_PATTERN_SIGNLE:
      singlePatternHandler();
      break;
    case LED_PATTERN_MULTIPLE:      
      multiplePatternHandler();
      break;
    default:
      break;
  }
}
