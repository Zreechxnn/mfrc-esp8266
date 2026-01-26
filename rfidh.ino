#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <time.h>
#include <vector>
#include "Settings.h"
#include "LCD.h"
#include "Globals.h"
#include "Utils.h"
#include "Network.h"
#include "API.h"
#include "WebHandler.h"

WiFiManager wm;
MFRC522 mfrc522(SS_PIN, RST_PIN);
ESP8266WebServer server(80);
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

unsigned long lastWifiCheck = 0;
bool isConnected = false;
bool timeConfigured = false;
unsigned long wifiLostStart = 0;
unsigned long lastReconnectAttempt = 0;
bool reconnectInProgress = false;

int resetAttempts = 0;
unsigned long lastResetAttempt = 0;

int currentMode = MODE_NORMAL;

// Variabel webStatus dihapus
String lastUID = "-";
String lastTime = "-";
String apiStatus = "-";
String apiMessage = "-";
String apiNamaKelas = "-";

std::vector<OfflineData> offlineQueue;
std::vector<CardHistory> tapHistory;

unsigned long uiTimer = 0;
bool uiOverride = false;

void setup() {
    offlineQueue.reserve(maxOfflineQueue);
    tapHistory.reserve(maxCooldownList);

    initLCD();
    showLcd("System Booting", "Harap Tunggu...");

    SPI.begin();
    mfrc522.PCD_Init();

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    showLcd("Connecting WiFi", "Please wait...");

    wm.setTimeout(180);

    if (!wm.autoConnect(ap_ssid, ap_password)) {
        showLcd("WiFi Failed", "Restarting...");
        delay(2000);
        ESP.restart();
    }

    MDNS.begin(mDNS_hostname);

    // Setup Web Server Minimalis
    server.on("/", HTTP_GET, handleRoot);     // Tampilkan form reset
    server.on("/resetwifi", HTTP_POST, handleResetWiFi); // Proses reset
    // Endpoint /status dihapus
    
    server.begin();

    showLcd("READY", "Tap Kartu Anda");

    resetAttempts = 0;
}

void loop() {
    MDNS.update();
    server.handleClient();

    handleWiFiConnection(millis());

    lcdLoop();

    if (uiOverride && millis() > uiTimer) {
        uiOverride = false;
        if (currentMode == MODE_NORMAL) {
            showLcd("READY", "Tap Kartu Anda");
        } else {
            showLcd("MODE ADMIN", "Tap Kartu Baru");
        }
    }

    static unsigned long lastQueueProcess = 0;
    if (millis() - lastQueueProcess > 5000) {
        processOfflineQueue();
        lastQueueProcess = millis();

        cleanupMemory();

        if (resetAttempts > 0 && (millis() - lastResetAttempt > RESET_COOLDOWN)) {
            resetAttempts = 0;
        }
    }

    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    String uid = readUID();

    if (isMasterCard(uid)) {
        if (currentMode == MODE_NORMAL) {
            currentMode = MODE_REGISTER;
            showLcd("MODE ADMIN", "Tap Kartu Baru");
        } else {
            currentMode = MODE_NORMAL;
            showLcd("MODE NORMAL", "Siap Tap");
        }

        uiOverride = true;
        uiTimer = millis() + 2000;

        mfrc522.PICC_HaltA();
        return;
    }

    if (currentMode == MODE_REGISTER) {
        registerCard(uid);
        uiOverride = true;
        uiTimer = millis() + 2000;
        mfrc522.PICC_HaltA();
        return;
    }

    String timestamp = getTimestamp();

    bool cooldown = false;
    for (const auto& history : tapHistory) {
        if (history.uid == uid && (millis() - history.lastTapTime < cooldownKartu)) {
            showLcd("TUNGGU...", "Masih Cooldown");
            uiOverride = true;
            uiTimer = millis() + 1500;
            mfrc522.PICC_HaltA();
            return;
        }
    }

    bool found = false;
    for (auto& history : tapHistory) {
        if (history.uid == uid) {
            history.lastTapTime = millis();
            found = true;
            break;
        }
    }
    if (!found) {
        if (tapHistory.size() >= maxCooldownList) {
            tapHistory.erase(tapHistory.begin());
        }
        tapHistory.push_back({uid, millis()});
    }

    lastUID = uid;
    lastTime = timestamp;

    sendData(lastUID, lastTime);

    uiOverride = true;
    uiTimer = millis() + 30000;

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}