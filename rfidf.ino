#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <time.h>

// =========================================
// URUTAN INCLUDE (JANGAN DIUBAH)
// =========================================
#include "Settings.h"
#include "LCD.h"
#include "Globals.h"
#include "Utils.h"     
#include "Network.h"
#include "API.h"
#include "WebHandler.h"

// =========================================
// DEFINISI OBJEK GLOBAL
// =========================================
WiFiManager wm;
MFRC522 mfrc522(SS_PIN, RST_PIN);
ESP8266WebServer server(80);
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

// =========================================
// DEFINISI VARIABEL GLOBAL
// =========================================
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

// =========================================
// VARIABEL NON-BLOCKING (FIX LINKER ERROR)
// =========================================
// Variabel UI
unsigned long uiTimer = 0;
bool uiOverride = false;

// Variabel Buzzer (Ini yang sebelumnya kurang)
int buzzerState = 0;
int buzzerStep = 0;
unsigned long buzzerTimer = 0;

// =========================================
// SETUP
// =========================================
void setup() {
    // 1. Init Hardware
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.begin(115200);

    // 2. Init LCD
    initLCD();
    showLcd("System Booting", "Harap Tunggu...");

    // 3. Init RFID
    SPI.begin();
    mfrc522.PCD_Init();

    // 4. Init WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    showLcd("Connecting WiFi", "Please wait...");
    
    // Set timeout agar alat tidak hang jika WiFi mati saat booting
    wm.setTimeout(180); 
    
    if (!wm.autoConnect(ap_ssid, ap_password)) {
        showLcd("WiFi Failed", "Restarting...");
        // Delay di setup aman karena loop belum jalan
        delay(2000); 
        ESP.restart();
    }

    // 5. Init Network Services
    MDNS.begin(mDNS_hostname);

    // 6. Init Web Server Routes
    server.on("/", handleRoot);
    server.on("/status", handleStatusJSON);
    server.on("/resetwifi", handleResetWiFi);
    server.begin();

    // 7. Selesai
    showLcd("READY", "Tap Kartu Anda");
    bunyiBuzzerSukses();
    
    cleanupMemory();
    resetAttempts = 0;
}

// =========================================
// LOOP UTAMA (NON-BLOCKING)
// =========================================
void loop() {
    // 1. Core Handlers (Wajib jalan terus agar responsif)
    MDNS.update();
    server.handleClient(); // Webserver handle request
    handleBuzzerLoop();    // Buzzer handle bunyi tanpa delay

    // 2. WiFi Management
    handleWiFiConnection(millis());

    // 3. LCD Scroll & UI Reset Logic
    lcdLoop();
    
    // Logic: Kembalikan tampilan LCD ke menu utama setelah pesan sementara selesai
    if (uiOverride && millis() > uiTimer) {
        uiOverride = false;
        if (currentMode == MODE_NORMAL) {
            showLcd("READY", "Tap Kartu Anda");
        } else {
            showLcd("MODE ADMIN", "Tap Kartu Baru");
        }
    }

    // 4. Offline Queue Processing (Setiap 5 detik)
    static unsigned long lastQueueProcess = 0;
    if (millis() - lastQueueProcess > 5000) {
        processOfflineQueue();
        lastQueueProcess = millis();
        
        cleanupMemory();
        
        // Reset counter password wifi jika sudah lewat cooldown
        if (resetAttempts > 0 && (millis() - lastResetAttempt > RESET_COOLDOWN)) {
            resetAttempts = 0;
        }
    }

    // 5. RFID Handling
    // Cek apakah ada kartu baru
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    String uid = readUID();

    // --- Master Card Logic ---
    if (uid == MASTER_CARD_UID) {
        if (currentMode == MODE_NORMAL) {
            currentMode = MODE_REGISTER;
            showLcd("MODE ADMIN", "Tap Kartu Baru");
            bunyiBuzzer();
        } else {
            currentMode = MODE_NORMAL;
            showLcd("MODE NORMAL", "Siap Tap");
            bunyiBuzzerSukses();
        }
        
        // Tahan pesan mode selama 2 detik sebelum bisa discroll/ditimpa
        uiOverride = true;
        uiTimer = millis() + 2000;
        
        mfrc522.PICC_HaltA();
        return;
    }

    // --- Register Mode ---
    if (currentMode == MODE_REGISTER) {
        // Fungsi registerCard ada di API.h
        registerCard(uid); 
        
        // Tampilkan hasil register selama 2 detik
        uiOverride = true;
        uiTimer = millis() + 2000;
        
        mfrc522.PICC_HaltA();
        return;
    }

    // --- Normal Mode ---
    
    // 1. Ambil Timestamp (Tanpa Validasi - Backend handle tanggal 2000/1970)
    String timestamp = getTimestamp(); 

    // 2. Cek Cooldown Kartu
    bool cooldown = false;
    for (auto& historyPtr : tapHistory) {
        if (historyPtr->uid == uid && (millis() - historyPtr->lastTapTime < cooldownKartu)) {
            showLcd("TUNGGU...", "Masih Cooldown");
            
            // Set pesan error tampil 1.5 detik
            uiOverride = true;
            uiTimer = millis() + 1500;
            
            bunyiBuzzer();
            mfrc522.PICC_HaltA();
            return;
        }
    }

    // 3. Update History Tap
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

    // 4. Kirim Data ke Server / Simpan Offline
    lastUID = uid;
    lastTime = timestamp;
    
    // Fungsi sendData ada di API.h (akan otomatis masuk queue jika offline)
    sendData(lastUID, lastTime); 
    
    // 5. Tampilkan status hasil kirim selama 3 detik
    uiOverride = true;
    uiTimer = millis() + 3000;

    // 6. Stop Reading
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}