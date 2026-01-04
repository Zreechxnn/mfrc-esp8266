#ifndef UTILS_H
#define UTILS_H

#include "Globals.h"

// Helper Functions
void bunyiBuzzer() {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
}

void bunyiBuzzerSukses() {
    digitalWrite(BUZZER_PIN, HIGH); delay(80);
    digitalWrite(BUZZER_PIN, LOW);  delay(50);
    digitalWrite(BUZZER_PIN, HIGH); delay(80);
    digitalWrite(BUZZER_PIN, LOW);
}

void bunyiBuzzerGagal() {
    digitalWrite(BUZZER_PIN, HIGH); delay(500);
    digitalWrite(BUZZER_PIN, LOW);
}

String readUID() {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
        uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    String formattedUID = "";
    for (int i = 0; i < uid.length(); i += 2) {
        if (i > 0) formattedUID += ":";
        formattedUID += uid.substring(i, i + 2);
    }
    return formattedUID;
}

String getTimestamp() {
    time_t now = time(nullptr);
    if (now < 100000) return "2000-01-01T00:00:00";
    struct tm* timeinfo = localtime(&now);
    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", timeinfo);
    return String(timestamp);
}

// Fungsi cleanupMemory tetap di sini
void cleanupMemory() {
    if (tapHistory.size() > maxCooldownList) {
        auto it = tapHistory.begin();
        std::advance(it, maxCooldownList);
        while (it != tapHistory.end()) {
            delete *it;
            it = tapHistory.erase(it);
        }
    }

    if (offlineQueue.size() > 50) {
        auto it = offlineQueue.begin();
        std::advance(it, 50);
        while (it != offlineQueue.end()) {
            delete *it;
            it = offlineQueue.erase(it);
        }
    }
}

#endif