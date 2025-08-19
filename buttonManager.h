#ifndef __BUTTON_MANAGER__
#define __BUTTON_MANAGER__

#define BUTTON_PIN 22
class ButtonManager {
  private:
    bool buttonPressed;
    unsigned long buttonPressTime;
    int buttonPin;
    bool isDbouce;

    void setButtonStatus(bool state);
    void readButtonStatus_dboubce();
  public:
    ButtonManager(int pin) : buttonPin(pin) {
      buttonPressed = false;
      buttonPressTime = 0;
      isDbouce=false;
    }

    void begin();

    bool isButtonPressed() ;
 
    void handleButtonPressedEvent();
};

#endif //__BUTTON_MANAGER__