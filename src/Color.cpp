/*
Joe Mulhern 2017
PWM colors for strips. Make with RGB LEDs
*/
#include "Color.h"

Color::Color(int redPIN, int greenPIN)
{
  _redPIN = redPIN;
  _greenPIN = greenPIN;
  pinMode(_redPIN, OUTPUT);
  pinMode(_greenPIN, OUTPUT);
}

void Color::whiteRed()
{
  analogWrite(_redPIN, 0);
  analogWrite(_greenPIN, 255);
}

void Color::whiteGreen()
{
  analogWrite(_redPIN, 255);
  analogWrite(_greenPIN, 0);
}

void Color::whiteBoth()
{
  analogWrite(_redPIN, 0);
  analogWrite(_greenPIN, 0);
}

void Color::off()
{
  analogWrite(_redPIN, 255); // 255 = Off
  analogWrite(_greenPIN, 255);
}

void Color::blink()
{
  int FADESPEED = 5;
  int FADE_LIMIT = 255;
  int FADE_OFFSET = 10; // Adjust if strip does not go to 0

  for (int r = 0; r < FADE_LIMIT + 1; r++)
  {
    analogWrite(_redPIN, FADE_LIMIT - r);
    delay(FADESPEED);
  }

  // fade from yellow to green
  for (int r = FADE_LIMIT + 1; r > 0; r--)
  {
    analogWrite(_redPIN, FADE_LIMIT - r + FADE_OFFSET);
    delay(FADESPEED);
  }

  // fade from red to yellow
  for (int g = 0; g < FADE_LIMIT + 1; g++)
  {
    analogWrite(_greenPIN, FADE_LIMIT - g);
    delay(FADESPEED);
  }

  // fade from yellow to green
  for (int r = FADE_LIMIT + 1; r > 0; r--)
  {
    analogWrite(_greenPIN, FADE_LIMIT - r + FADE_OFFSET);
    delay(FADESPEED);
  }
}

void Color::blinkShort()
{
  int FADESPEED = 1; // Lower= Faster... Higher=Slower
  int FADE_LIMIT = 255;
  int FADE_OFFSET = 10; // Adjust if strip does not go to 0

  for (int r = 0; r < FADE_LIMIT + 1; r++)
  {
    analogWrite(_redPIN, FADE_LIMIT - r);
    delay(FADESPEED);
  }

  // fade from yellow to green
  for (int r = FADE_LIMIT + 1; r > 0; r--)
  {
    analogWrite(_redPIN, FADE_LIMIT - r + FADE_OFFSET);
    delay(FADESPEED);
  }

  // fade from red to yellow
  for (int g = 0; g < FADE_LIMIT + 1; g++)
  {
    analogWrite(_greenPIN, FADE_LIMIT - g);
    delay(FADESPEED);
  }

  // fade from yellow to green
  for (int r = FADE_LIMIT + 1; r > 0; r--)
  {
    analogWrite(_greenPIN, FADE_LIMIT - r + FADE_OFFSET);
    delay(FADESPEED);
  }
}