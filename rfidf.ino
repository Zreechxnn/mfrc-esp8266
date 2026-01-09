#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <time.h>

#include "Settings.h"
#include "LCD.h"
#include "Globals.h"
#include "Utils.h"
#include "Network.h"
#include "API.h"
#include "WebHandler.h"

// =========================================
//OBJEK GLOBAL
// =========================================
WiFiManager wm;
MFRC522 mfrc522(SS_PIN, RST_PIN);
ESP8266WebServer server(80);
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

// =========================================
// VARIABEL GLOBAL
// =========================================
unsigned long lastWifiCheck = 0;
bool isConnected = false;
bool timeConfigured = false;
unsigned long wifiLostStart = 0;
unsigned long lastReconnectAttempt = 0;
bool reconnectInProgress = false;

int resetAttempts = 0;
unsigned long lastResetAttempt = 0;

// Current Mode
int currentMode = MODE_NORMAL;

String webStatus = "Sistem Siap.";
String lastUID = "-";
String lastTime = "-";
String apiStatus = "-";
String apiMessage = "-";
String apiNamaKelas = "-";

std::list<OfflineData*> offlineQueue;
std::list<CardHistory*> tapHistory;

unsigned long uiTimer = 0;
bool uiOverride = false;

void setup() {

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

    server.on("/", handleRoot);
    server.on("/status", handleStatusJSON);
    server.on("/resetwifi", handleResetWiFi);
    server.begin();

    showLcd("READY", "Tap Kartu Anda");

    cleanupMemory();
    resetAttempts = 0;
}

void loop() {
    MDNS.update();
    server.handleClient(); // Webserver handle request

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

    // --- Master Card Logic ---
    if (isMasterCard(uid)) {
        if (currentMode == MODE_NORMAL) {
            currentMode = MODE_REGISTER;
            showLcd("MODE ADMIN", "Tap Kartu Baru");
        } else {
            currentMode = MODE_NORMAL;
            showLcd("MODE NORMAL", "Siap Tap");
        }

        // Tahan pesan mode selama 2 detik
        uiOverride = true;
        uiTimer = millis() + 2000;

        mfrc522.PICC_HaltA();
        return;
    }

    // --- Register Mode ---
    if (currentMode == MODE_REGISTER) {
        registerCard(uid);

        uiOverride = true;
        uiTimer = millis() + 2000;

        mfrc522.PICC_HaltA();
        return;
    }

    String timestamp = getTimestamp();

    bool cooldown = false;
    for (auto& historyPtr : tapHistory) {
        if (historyPtr->uid == uid && (millis() - historyPtr->lastTapTime < cooldownKartu)) {
            showLcd("TUNGGU...", "Masih Cooldown");

            uiOverride = true;
            uiTimer = millis() + 1500;

            mfrc522.PICC_HaltA();
            return;
        }
    }

    bool found = false;
    for (auto& historyPtr : tapHistory) {
        if (historyPtr->uid == uid) {
            historyPtr->lastTapTime = millis();
            found = true;
            break;
        }
    }
    if (!found) {
        if (tapHistory.size() >= maxCooldownList) {
            delete tapHistory.front();
            tapHistory.pop_front();
        }
        tapHistory.push_back(new CardHistory{uid, millis()});
    }

    lastUID = uid;
    lastTime = timestamp;

    sendData(lastUID, lastTime);

    uiOverride = true;
    uiTimer = millis() + 3000;

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}