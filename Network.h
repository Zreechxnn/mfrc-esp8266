#ifndef NETWORK_H
#define NETWORK_H

#include "Globals.h"
#include "Utils.h"

void handleWiFiConnection(unsigned long currentMillis) {
    static int reconnectCount = 0;
    
    if (WiFi.status() != WL_CONNECTED) {
        isConnected = false;
        timeConfigured = false;
        
        if (wifiLostStart == 0) {
            wifiLostStart = currentMillis;
            showLcd("WiFi TERPUTUS", "Mencoba sambung...");
        }
        
        // Coba reconnect setiap 15 detik
        if (currentMillis - lastReconnectAttempt >= wifiReconnectInterval && !reconnectInProgress) {
            lastReconnectAttempt = currentMillis;
            reconnectInProgress = true;
            reconnectCount++;
            
            showLcd("Reconnecting", "Attempt: " + String(reconnectCount));
            
            // Coba reconnect dengan WiFi.begin
            WiFi.disconnect();
            delay(100);
            WiFi.mode(WIFI_STA);
            WiFi.begin();
            
            // Tunggu koneksi dengan timeout 5 detik
            unsigned long startAttemptTime = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
                delay(100);
            }
            
            if (WiFi.status() == WL_CONNECTED) {
                showLcd("WiFi Connected", WiFi.localIP().toString());
                reconnectCount = 0;
                wifiLostStart = 0;
                reconnectInProgress = false;
                bunyiBuzzerSukses();
                return;
            }
            
            reconnectInProgress = false;
            
            // Jika sudah 3x gagal reconnect, buka config portal
            if (reconnectCount >= 3) {
                bunyiBuzzer();
                showLcd("Setup Portal", "Aktif...");
                
                WiFiManager wm;
                wm.setConfigPortalTimeout(180);
                
                if (!wm.startConfigPortal(ap_ssid, ap_password)) {
                    showLcd("Restarting...", "Please wait");
                    delay(2000);
                    ESP.restart();
                } else {
                    showLcd("WiFi Connected", "Portal Success");
                    reconnectCount = 0;
                    wifiLostStart = 0;
                }
            }
        }
        
        // Blink LED/buzzer saat offline
        if (currentMillis % 2000 < 100) {
            digitalWrite(BUZZER_PIN, HIGH);
        } else {
            digitalWrite(BUZZER_PIN, LOW);
        }
    } else {
        // Connected
        if (!isConnected) {
            isConnected = true;
            wifiLostStart = 0;
            reconnectCount = 0;
            reconnectInProgress = false;
            digitalWrite(BUZZER_PIN, LOW);
            
            showLcd("WiFi Connected", WiFi.localIP().toString());
            bunyiBuzzerSukses();
        }
        
        if (!timeConfigured) {
            configTime(timeZoneOffset, dst, "id.pool.ntp.org", "pool.ntp.org", "time.nist.gov");
            timeConfigured = true;
        }
    }
}

#endif