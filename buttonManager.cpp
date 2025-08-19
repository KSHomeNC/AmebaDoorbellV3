#include <type_traits>
#include <cstddef>

#include <Arduino.h>
#include "buttonManager.h"
#include <GTimer.h>

#define TIMER_ID 0

void time_handler(uint32_t data);

ButtonManager *bMgr=NULL;

void timer_handler(uint32_t data)
{   
  bMgr->handleButtonPressedEvent();
}
void ButtonManager:: handleButtonPressedEvent() {
  if ( (digitalRead(buttonPin) == HIGH) && (isDbouce == false)) {
	  
    isDbouce = true;
  }else if ( (digitalRead(buttonPin) == HIGH) && (isDbouce == true) ){
    setButtonStatus(true); 
    isDbouce= false;
    //GTimer.reload(TIMER_ID, (1 * 1000 * 1000)); //every second    
  }else{
    //GTimer.reload(TIMER_ID, (1 * 1000 * 1000)); //every second   
    isDbouce= false; 
  }
  GTimer.reload(TIMER_ID, (100 * 1000)); // 100 mSec debaunce period
}
void ButtonManager:: begin() {
  pinMode(buttonPin, INPUT);  
  bMgr = this;
  GTimer.begin(TIMER_ID, (2 * 1000 * 1000), timer_handler); // first for 2 Sec at boot up time
}

bool ButtonManager:: isButtonPressed() {
  bool state;
 
  state = buttonPressed;
  buttonPressed = false; // reset the value for next cycle
  return state;
}

void ButtonManager:: readButtonStatus_dboubce() {
  if (digitalRead(buttonPin) == HIGH && !buttonPressed) {
	  buttonPressed = true;
	  buttonPressTime = millis();	
  }
  if (buttonPressed && millis() - buttonPressTime >= 10000) {
	  buttonPressed = false;
  } 
}

void ButtonManager:: setButtonStatus(bool state) {
  buttonPressed = state;
}
