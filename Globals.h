#ifndef GLOBALS_H
#define GLOBALS_H

#include "Settings.h"
#include "LCD.h"  // Tambahkan include untuk LCD

// Manager Objects
extern WiFiManager wm;
extern MFRC522 mfrc522;
extern ESP8266WebServer server;
extern LiquidCrystal_I2C lcd;

// Global Variables
extern unsigned long lastWifiCheck;
extern bool isConnected;
extern bool timeConfigured;
extern unsigned long wifiLostStart;
extern unsigned long lastReconnectAttempt;
extern bool reconnectInProgress;

// Security Variables
extern int resetAttempts;
extern unsigned long lastResetAttempt;

// Current Mode
extern int currentMode;

// Status Strings
extern String webStatus;
extern String lastUID;
extern String lastTime;
extern String apiStatus;
extern String apiMessage;
extern String apiNamaKelas;

// Data Structures
struct OfflineData {
    String uid;
    String timestamp;
};

struct CardHistory {
    String uid;
    unsigned long lastTapTime;
};

// Menggunakan list untuk efisiensi memory dan operasi push/pop
extern std::list<OfflineData*> offlineQueue;
extern std::list<CardHistory*> tapHistory;

// Function Prototypes dari file lain
void handleWiFiConnection(unsigned long currentMillis);
void cleanupMemory();
bool verifyResetPassword(String password);

#endif