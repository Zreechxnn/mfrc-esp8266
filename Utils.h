#ifndef UTILS_H
#define UTILS_H

#include "Globals.h"

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

    if (now < 1000000000) {
        return "";
    }

    struct tm* timeinfo = localtime(&now);
    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", timeinfo);
    return String(timestamp);
}

void cleanupMemory() {
    if (tapHistory.size() > maxCooldownList) {
        auto it = tapHistory.begin();
        std::advance(it, maxCooldownList);
        while (it != tapHistory.end()) {
            it = tapHistory.erase(it);
        }
    }
    if (offlineQueue.size() > 50) {
        auto it = offlineQueue.begin();
        std::advance(it, 50);
        while (it != offlineQueue.end()) {
            it = offlineQueue.erase(it);
        }
    }
}

void setTemporaryMessage(String l1, String l2, int duration) {
    showLcd(l1, l2);
    uiOverride = true;
    uiTimer = millis() + duration;
}

#endif