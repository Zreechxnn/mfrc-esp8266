#ifndef SETTINGS_H
#define SETTINGS_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <MFRC522.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <time.h>
#include <vector>
#include <list>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <LiquidCrystal_I2C.h>

// Deklarasi eksternal (hanya deklarasi, bukan definisi)
extern const char* mDNS_hostname;
extern const char* ap_ssid;
extern const char* ap_password;
extern const char* serverURL;
extern const char* registerURL;
extern const int ID_RUANGAN;

extern const String MASTER_CARD_UID;
extern const String RESET_WIFI_PASSWORD;
extern const int MAX_RESET_ATTEMPTS;
extern const unsigned long RESET_COOLDOWN;

// Modes (bukan variabel, jadi bisa #define)
#define MODE_NORMAL 0
#define MODE_REGISTER 1

// Pin Definitions (bukan variabel, jadi bisa #define)
#define SS_PIN D4
#define RST_PIN D3
#define BUZZER_PIN D0

// LCD Configuration (bukan variabel, jadi bisa #define)
#define LCD_I2C_ADDRESS 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// Time Configuration (bukan variabel, bisa constexpr)
constexpr int timeZoneOffset = 7 * 3600; // GMT+7
constexpr int dst = 0;

// Timing Configuration (bukan variabel, bisa constexpr)
constexpr unsigned long debounceDelay = 1000;
constexpr unsigned long cooldownKartu = 60000;
constexpr unsigned long wifiReconnectInterval = 15000; // 15 detik
constexpr int maxCooldownList = 20;

#endif