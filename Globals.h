#ifndef GLOBALS_H
#define GLOBALS_H

#include "Settings.h"
#include "LCD.h"

extern WiFiManager wm;
extern MFRC522 mfrc522;
extern ESP8266WebServer server;
extern LiquidCrystal_I2C lcd;

extern unsigned long lastWifiCheck;
extern bool isConnected;
extern bool timeConfigured;
extern unsigned long wifiLostStart;
extern unsigned long lastReconnectAttempt;
extern bool reconnectInProgress;

extern int resetAttempts;
extern unsigned long lastResetAttempt;

extern int currentMode;

extern String webStatus;
extern String lastUID;
extern String lastTime;
extern String apiStatus;
extern String apiMessage;
extern String apiNamaKelas;

extern unsigned long uiTimer; 
extern bool uiOverride;  

struct OfflineData {
    String uid;
    String timestamp;
};

struct CardHistory {
    String uid;
    unsigned long lastTapTime;
};

extern std::list<OfflineData*> offlineQueue;
extern std::list<CardHistory*> tapHistory;

void handleWiFiConnection(unsigned long currentMillis);
void cleanupMemory();
bool verifyResetPassword(String password);

#endif