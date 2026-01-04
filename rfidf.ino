#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <time.h>

// Include headers - PERHATIKAN URUTANNYA
#include "Settings.h"
#include "LCD.h"
#include "Globals.h"
#include "Utils.h"
#include "Network.h"
#include "API.h"
#include "WebHandler.h"

// Definisi objek global
WiFiManager wm;
MFRC522 mfrc522(SS_PIN, RST_PIN);
ESP8266WebServer server(80);
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

// Definisi variabel global
unsigned long lastWifiCheck = 0;
bool isConnected = false;
bool timeConfigured = false;
unsigned long wifiLostStart = 0;
unsigned long lastReconnectAttempt = 0;
bool reconnectInProgress = false;

// Security Variables
int resetAttempts = 0;
unsigned long lastResetAttempt = 0;

// Current Mode
int currentMode = MODE_NORMAL;

// Status Strings
String webStatus = "Sistem Siap.";
String lastUID = "-";
String lastTime = "-";
String apiStatus = "-";
String apiMessage = "-";
String apiNamaKelas = "-";

// Data Structures
std::list<OfflineData*> offlineQueue;
std::list<CardHistory*> tapHistory;

void setup() {
    // Initialize hardware
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.begin(115200);

    // Initialize LCD
    initLCD();
    showLcd("System Booting", "Harap Tunggu...");

    // Initialize RFID
    SPI.begin();
    mfrc522.PCD_Init();

    // Initialize WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    // Try to connect to saved network
    showLcd("Connecting WiFi", "Please wait...");

    WiFiManager wm;
    wm.setTimeout(180);

    if (!wm.autoConnect(ap_ssid, ap_password)) {
        showLcd("WiFi Failed", "Restarting...");
        delay(2000);
        ESP.restart();
    }

    // Initialize mDNS
    MDNS.begin(mDNS_hostname);

    // Initialize web server
    server.on("/", handleRoot);
    server.on("/status", handleStatusJSON);
    server.on("/resetwifi", handleResetWiFi);
    server.begin();

    showLcd("READY", "Tap Kartu Anda");
    bunyiBuzzerSukses();

    // Initial memory cleanup
    cleanupMemory();

    // Initial status
    resetAttempts = 0;
}

void loop() {
    MDNS.update();
    server.handleClient();

    // Handle WiFi connection dengan reconnection setiap 15 detik
    handleWiFiConnection(millis());

    // Handle LCD scrolling
    lcdLoop();

    // Process offline queue every 5 seconds if online
    static unsigned long lastQueueProcess = 0;
    if (millis() - lastQueueProcess > 5000) {
        processOfflineQueue();
        lastQueueProcess = millis();

        // Periodic memory cleanup
        cleanupMemory();

        // Reset attempt counter setelah cooldown
        if (resetAttempts > 0 && (millis() - lastResetAttempt > RESET_COOLDOWN)) {
            resetAttempts = 0;
        }
    }

    // Check for RFID card
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    String uid = readUID();

    // Master Card Logic
    if (uid == MASTER_CARD_UID) {
        if (currentMode == MODE_NORMAL) {
            currentMode = MODE_REGISTER;
            showLcd("MODE ADMIN", "Tap Kartu Baru");
            bunyiBuzzer();
            delay(1000);
        } else {
            currentMode = MODE_NORMAL;
            showLcd("MODE NORMAL", "Siap Tap");
            bunyiBuzzerSukses();
            delay(1000);
        }
        mfrc522.PICC_HaltA();
        return;
    }

    // Register Mode - HANYA 1 TAP lalu kembali normal
    if (currentMode == MODE_REGISTER) {
        registerCard(uid);
        delay(1500); // Tampilkan hasil register

        // Otomatis kembali ke mode normal
        currentMode = MODE_NORMAL;
        showLcd("READY", "Tap Kartu Anda");
        mfrc522.PICC_HaltA();
        return;
    }

    // Normal Mode - Check cooldown dengan pointer
    bool cooldown = false;
    for (auto& historyPtr : tapHistory) {
        if (historyPtr->uid == uid && (millis() - historyPtr->lastTapTime < cooldownKartu)) {
            showLcd("TUNGGU...", "Masih Cooldown");
            bunyiBuzzer();
            mfrc522.PICC_HaltA();
            return;
        }
    }

    // Update history dengan pointer
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
            // Hapus history tertua
            delete tapHistory.front();
            tapHistory.pop_front();
        }

        CardHistory* newHistory = new CardHistory{uid, millis()};
        tapHistory.push_back(newHistory);
    }

    // Send data
    lastUID = uid;
    lastTime = getTimestamp();
    sendData(lastUID, lastTime);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}