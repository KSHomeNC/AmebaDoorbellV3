#ifndef __LED_MANAGER__
#define __LED_MANAGER__

#define BLUE_LED_PIN 19
#define GREEN_LED_PIN 20
#define RED_LED_PIN 21

#define MAX_LEDS 3 // blue, green, red
/*
* onDuration  offDuration
    0           0            always OFF
    0           1            always OFF
    1           0            always On
    1           1            toggling 
*/
struct LEDinfo {   
  bool defaultState; // 0 = OFF , 1=ON
  unsigned long onDuration;  //0 off
  unsigned long offDuration; //0 on but onDuration must be 1 else off
};

struct LEDCntrlArray{
  int ledPin;
  LEDinfo led;  
  unsigned long startTime; // in millis
  unsigned long duration; // in millis
  bool ledState;
};

enum LEDColor {
  BLUE = 0,
  GREEN = 1,
  RED = 2,
  LED_END =3
};


enum LED_PATTERN_TYPE {
  LED_PATTERN_NONE = 0,
  LED_PATTERN_SIGNLE = 1,
  LED_PATTERN_MULTIPLE = 2,
  LED_PATTERN_UNDEF = 3  
};

class LEDManager {
  private:
    LEDCntrlArray ledArray[MAX_LEDS];
    
    LED_PATTERN_TYPE currentLedPatternState;
    LED_PATTERN_TYPE PreviousLedPatternState;
    
    unsigned long multipleLedToggleDuration; 
    unsigned long multipleLedToggleStartT; 
    int multipleLedCnt;
    int multipleLedloopStartT;
    bool onTime;
    
    bool isLedTimerExpire(unsigned long startT, unsigned long duration );
    void singlePatternHandler();
    void multiplePatternHandler();
    void nonePatternHandler();
    void updateLEDPattern() ;
    

  public:
    LEDManager() ;
    void begin() ;
    
    void setNextPatternMultiple(unsigned long duration);
    void setNextPatternSingle( LEDColor led, LEDinfo* ledinfo);
    void setNextPatternNone( LEDColor led);
    
    void loop();
};
#endif //__LED_MANAGER__
