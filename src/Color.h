/*
Joe Mulhern 2017
PWM colors for strips. Make with RGB LEDs
*/
#ifndef Color_h
#define Color_h

#include "Arduino.h"

class Color
{
public:
  Color(int redPIN, int greenPIN);
  void whiteRed();
  void whiteGreen();
  void whiteBoth();
  void off();
  void blink();
  void blinkShort();

private:
  int _redPIN, _greenPIN;
};

#endif
