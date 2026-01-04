#include "LCD.h"
#include "Settings.h"

// Definisi konstanta scroll (hanya di sini)
const int SCROLL_SPEED = 400; // ms per karakter
const int SCROLL_PAUSE = 2000; // ms jeda sebelum scroll ulang

// Definisi variabel scrolling
String lcdLine1 = "";
String lcdLine2 = "";
int scrollIdx1 = 0;
int scrollIdx2 = 0;
unsigned long lastLcdUpdate = 0;
unsigned long scrollStartTime1 = 0;
unsigned long scrollStartTime2 = 0;
bool scrollActive1 = false;
bool scrollActive2 = false;

// Inisialisasi LCD
void initLCD() {
    Wire.begin(D2, D1);
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

// Fungsi untuk menampilkan teks di LCD dengan format
void showLcd(String l1, String l2, bool forceUpdate) {
    lcdLine1 = l1;
    lcdLine2 = l2;

    scrollIdx1 = 0;
    scrollIdx2 = 0;
    scrollActive1 = (l1.length() > LCD_COLS);
    scrollActive2 = (l2.length() > LCD_COLS);

    if (scrollActive1) {
        scrollStartTime1 = millis() + SCROLL_PAUSE;
    }
    if (scrollActive2) {
        scrollStartTime2 = millis() + SCROLL_PAUSE;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(l1.length() > LCD_COLS ? l1.substring(0, LCD_COLS) : l1);
    lcd.setCursor(0, 1);
    lcd.print(l2.length() > LCD_COLS ? l2.substring(0, LCD_COLS) : l2);

    lastLcdUpdate = millis();
}

// Fungsi untuk menampilkan teks terpusat
void showLcdCentered(String text, int row) {
    lcd.setCursor(0, row);
    lcd.print("                "); // Clear line
    int pos = (LCD_COLS - text.length()) / 2;
    lcd.setCursor(max(0, pos), row);
    lcd.print(text);
}

// Fungsi khusus untuk menampilkan status dengan nama kelas dari API
void showLcdWithClass(String status, String className) {
    String line1 = status;
    String line2 = className;
    
    // Jika kelas terlalu panjang, potong atau format
    if (className.length() > LCD_COLS) {
        line2 = className.substring(0, LCD_COLS);
    }
    
    showLcd(line1, line2);
}

// Fungsi untuk mendapatkan teks scroll
String getScrollText(String text, int &idx, unsigned long &scrollStart, bool &active) {
    if (!active) return text;

    unsigned long currentMillis = millis();
    if (currentMillis < scrollStart) {
        return text.substring(0, LCD_COLS);
    }

    int textLength = text.length();
    if (textLength <= LCD_COLS) {
        active = false;
        return text;
    }

    String displayText = text + "    ";
    int displayLength = displayText.length();

    if (idx >= displayLength) {
        idx = 0;
        scrollStart = currentMillis + SCROLL_PAUSE;
        return text.substring(0, LCD_COLS);
    }

    String result = displayText.substring(idx, min(idx + LCD_COLS, displayLength));
    if (result.length() < LCD_COLS) {
        result += displayText.substring(0, LCD_COLS - result.length());
    }

    return result;
}

// Loop untuk menangani scrolling LCD
void lcdLoop() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastLcdUpdate > SCROLL_SPEED) {
        lastLcdUpdate = currentMillis;

        bool updateNeeded = false;

        if (scrollActive1) {
            String line1 = getScrollText(lcdLine1, scrollIdx1, scrollStartTime1, scrollActive1);
            lcd.setCursor(0, 0);
            lcd.print(line1);
            if (scrollActive1) {
                scrollIdx1++;
                updateNeeded = true;
            }
        }

        if (scrollActive2) {
            String line2 = getScrollText(lcdLine2, scrollIdx2, scrollStartTime2, scrollActive2);
            lcd.setCursor(0, 1);
            lcd.print(line2);
            if (scrollActive2) {
                scrollIdx2++;
                updateNeeded = true;
            }
        }

        if (updateNeeded) {
            lastLcdUpdate = currentMillis;
        }
    }
}

// Fungsi untuk membersihkan LCD
void clearLCD() {
    lcd.clear();
    lcdLine1 = "";
    lcdLine2 = "";
    scrollActive1 = false;
    scrollActive2 = false;
}