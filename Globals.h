#ifndef GLOBALS_H
#define GLOBALS_H

#include "Settings.h"
#include "LCD.h"

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

// --- TAMBAHAN UNTUK NON-BLOCKING ---
extern unsigned long uiTimer;     // Timer untuk kembalikan tampilan LCD ke idle
extern bool uiOverride;           // Status apakah LCD sedang menampilkan pesan sementara
extern int buzzerState;           // 0=Mati, 1=Beep Pendek, 2=Sukses, 3=Gagal
extern int buzzerStep;            // Langkah dalam urutan bunyi
extern unsigned long buzzerTimer; // Timer untuk durasi bunyi
// ------------------------------------

// Data Structures
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