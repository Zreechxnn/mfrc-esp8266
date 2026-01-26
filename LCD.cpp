#include "LCD.h"
#include <Arduino.h>  // <--- TAMBAHAN PENTING: Agar D1 dan D2 dikenali

const int SCROLL_DELAY = 350;
const int PAUSE_DELAY = 2000;
const int GAP_SPACE = 4;

String line1, line2;

int idx1 = 0, idx2 = 0;
unsigned long nextTime1 = 0, nextTime2 = 0;
bool scroll1 = false, scroll2 = false;

char lineBuffer[17];

void initLCD() {
    Wire.begin(D2, D1);
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

void printLine(int row, const String& text, int startIndex) {
    int len = text.length();
    int totalLen = len + GAP_SPACE;
    for (int i = 0; i < 16; i++) {
        int ptr = (startIndex + i) % totalLen;

        if (ptr < len) {
            lineBuffer[i] = text[ptr];
        } else {
            lineBuffer[i] = ' ';
        }
    }
    lineBuffer[16] = '\0';

    lcd.setCursor(0, row);
    lcd.print(lineBuffer);
}

void showLcd(const String& l1, const String& l2) {
    if (line1 == l1 && line2 == l2) return;

    line1 = l1;
    line2 = l2;

    idx1 = 0;
    idx2 = 0;
    scroll1 = (line1.length() > 16);
    scroll2 = (line2.length() > 16);
    nextTime1 = millis() + PAUSE_DELAY;
    nextTime2 = millis() + PAUSE_DELAY;

    lcd.clear();

    if (scroll1) printLine(0, line1, 0);
    else { lcd.setCursor(0, 0); lcd.print(line1); }

    if (scroll2) printLine(1, line2, 0);
    else { lcd.setCursor(0, 1); lcd.print(line2); }
}

void updateScroll(int row, const String& text, int& idx, unsigned long& timer, bool active) {
    if (!active) return;
    if (millis() < timer) return;

    idx++;
    int totalLen = text.length() + GAP_SPACE;

    if (idx >= totalLen) {
        idx = 0;
        timer = millis() + PAUSE_DELAY;
    } else {
        timer = millis() + SCROLL_DELAY;
    }

    printLine(row, text, idx);
}

void lcdLoop() {
    updateScroll(0, line1, idx1, nextTime1, scroll1);
    updateScroll(1, line2, idx2, nextTime2, scroll2);
}

void clearLCD() {
    line1 = "";
    line2 = "";
    scroll1 = false;
    scroll2 = false;
    lcd.clear();
}