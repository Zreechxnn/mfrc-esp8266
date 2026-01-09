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

        if (currentMillis - lastReconnectAttempt >= wifiReconnectInterval && !reconnectInProgress) {
            lastReconnectAttempt = currentMillis;
            reconnectInProgress = true;
            reconnectCount++;

            showLcd("Reconnecting", "Attempt: " + String(reconnectCount));

            WiFi.disconnect();
            delay(100);
            WiFi.mode(WIFI_STA);
            WiFi.begin();

            unsigned long startAttemptTime = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
                delay(100);
            }

            if (WiFi.status() == WL_CONNECTED) {
                showLcd("WiFi Connected", WiFi.localIP().toString());
                reconnectCount = 0;
                wifiLostStart = 0;
                reconnectInProgress = false;
                return;
            }

            reconnectInProgress = false;

            if (reconnectCount >= 3) {
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
        
    } else {
        if (!isConnected) {
            isConnected = true;
            wifiLostStart = 0;
            reconnectCount = 0;
            reconnectInProgress = false;

            showLcd("WiFi Connected", WiFi.localIP().toString());
            // Buzzer sukses dihapus
        }

        if (!timeConfigured) {
            configTime(timeZoneOffset, dst, "id.pool.ntp.org", "pool.ntp.org", "time.nist.gov");
            timeConfigured = true;
        }
    }
}

#endif