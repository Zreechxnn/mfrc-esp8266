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

// Deklarasi eksternal
extern const char* mDNS_hostname;
extern const char* ap_ssid;
extern const char* ap_password;
extern const char* serverURL;
extern const char* registerURL;
extern const int ID_RUANGAN;

bool isMasterCard(String uid); 

extern const String RESET_WIFI_PASSWORD;
extern const int MAX_RESET_ATTEMPTS;
extern const unsigned long RESET_COOLDOWN;

// Modes
#define MODE_NORMAL 0
#define MODE_REGISTER 1

// Pin Definitions
#define SS_PIN D4
#define RST_PIN D3

// LCD Configuration
#define LCD_I2C_ADDRESS 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// Time Configuration
constexpr int timeZoneOffset = 7 * 3600; // GMT+7
constexpr int dst = 0;

// Timing Configuration
constexpr unsigned long debounceDelay = 1000;
constexpr unsigned long cooldownKartu = 60000;
constexpr unsigned long wifiReconnectInterval = 15000;
constexpr int maxCooldownList = 20;

#endif