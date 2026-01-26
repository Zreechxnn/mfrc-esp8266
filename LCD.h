#ifndef LCD_H
#define LCD_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

void initLCD();
void showLcd(const String& l1, const String& l2);
void lcdLoop();
void clearLCD();

#endif