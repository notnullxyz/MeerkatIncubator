/*
   display.cpp - library for GreenButler specific display control
   Copyright Marlon van der Linde <marlon250f@gmail.com> 2018
*/
#include <LiquidCrystal.h>
#include "Arduino.h"
#include "Display.h"
#include "parameters.h"

/**
   lcd parameters:
   rs, enable, d4, d5, d6, d7
*/
Display::Display() : lcd(PIN_LCD_RS, PIN_LCD_ENABLE, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7) {
  lcd.begin(LCD_CHARS, LCD_LINES);
}

void Display::begin() {
  lcd.home();
  lcd.print(SYS_GREETING);
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print(SYS_NAME_VERSION);
  delay(2000);
}

void Display::reset() {
  lcd.clear();
  lcd.home();
}

void Display::mute() {
  if (muted)
    lcd.display();
  else
    lcd.noDisplay();
  muted = !muted;
}

void Display::updateStatuses(bool lamp, bool humidifier, bool fan) {
  char *lampVal = (lamp == true) ? "on" : "off";
  char *humidifierVal = (humidifier == true) ? "on" : "off";
  char *fanVal = (fan == true) ? "on" : "off";

  // Set label positions, print labels.
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(NAME_LAMP);
  
  lcd.setCursor(0,1);
  lcd.print(NAME_HUMIDIFIER);

  lcd.setCursor(9,0);
  lcd.print(NAME_FAN);

  // set value positions and print values
  lcd.setCursor(5, 0);
  lcd.print(lampVal);
  
  lcd.setCursor(11, 1);
  lcd.print(humidifierVal);

  lcd.setCursor(13, 0);
  lcd.print(fanVal);
}

void Display::updateSensorReadings(float temp, float humid, char* realdate, char* realtime) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print((int)temp);
  lcd.setCursor(2,0);
  lcd.print((char)223); // degrees symbol
  
  lcd.setCursor(4,0);
  lcd.print((char)64);  // @ symbol
  lcd.setCursor(6,0);
  lcd.print((int)humid);
  lcd.setCursor(8,0);
  lcd.print('%');

  lcd.setCursor(10,0);
  lcd.print("Day");
  // todo, print day number here.
  lcd.setCursor(14,0);
  lcd.print("99");


  lcd.setCursor(0,1);
  lcd.print(realdate);

  lcd.setCursor(10,1);
  lcd.print(realtime);  
}

void Display::showReadingIndicator() {
  lcd.setCursor(15,1);
  lcd.print("*");  
}

void Display::clearReadingIndicator() {
  lcd.setCursor(16,1);
  lcd.print(" ");  
}

