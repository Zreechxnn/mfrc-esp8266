#ifndef LCD_H
#define LCD_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Deklarasi eksternal
extern LiquidCrystal_I2C lcd;

// Deklarasi variabel untuk scrolling LCD
extern String lcdLine1;
extern String lcdLine2;
extern int scrollIdx1;
extern int scrollIdx2;
extern unsigned long lastLcdUpdate;
extern unsigned long scrollStartTime1;
extern unsigned long scrollStartTime2;
extern bool scrollActive1;
extern bool scrollActive2;

// Deklarasi konstanta scroll
extern const int SCROLL_SPEED;
extern const int SCROLL_PAUSE;

// Function Prototypes
void showLcd(String l1, String l2, bool forceUpdate = false);
void showLcdCentered(String text, int row);
void showLcdWithClass(String status, String className);
String getScrollText(String text, int &idx, unsigned long &scrollStart, bool &active);
void lcdLoop();
void initLCD();
void clearLCD();

#endif