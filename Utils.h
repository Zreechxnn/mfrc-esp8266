#ifndef UTILS_H
#define UTILS_H

#include "Globals.h"

// --- LOGIKA BUZZER (TETAP SAMA) ---
void handleBuzzerLoop() {
    unsigned long now = millis();
    if (buzzerState == 0) return;

    if (buzzerState == 1)
    { // Beep Pendek
        if (buzzerStep == 0) {
            digitalWrite(BUZZER_PIN, HIGH);
            buzzerTimer = now;
            buzzerStep = 1;
        } else if (buzzerStep == 1 && now - buzzerTimer > 100) {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerState = 0;
            buzzerStep = 0;
        }
    }
    else if (buzzerState == 2)
    { // Beep Sukses
        if (buzzerStep == 0) {
            digitalWrite(BUZZER_PIN, HIGH);
            buzzerTimer = now;
            buzzerStep = 1;
        } else if (buzzerStep == 1 && now - buzzerTimer > 80) {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerTimer = now;
            buzzerStep = 2;
        } else if (buzzerStep == 2 && now - buzzerTimer > 50) {
            digitalWrite(BUZZER_PIN, HIGH);
            buzzerTimer = now;
            buzzerStep = 3;
        } else if (buzzerStep == 3 && now - buzzerTimer > 80) {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerState = 0;
            buzzerStep = 0;
        }
    }
    else if (buzzerState == 3)
    { // Beep Gagal
        if (buzzerStep == 0) {
            digitalWrite(BUZZER_PIN, HIGH);
            buzzerTimer = now;
            buzzerStep = 1;
        }
        else if (buzzerStep == 1 && now - buzzerTimer > 800)
        {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerState = 0;
            buzzerStep = 0;
        }
    }
}

void bunyiBuzzer()
{
    buzzerState = 1;
    buzzerStep = 0;
}
void bunyiBuzzerSukses()
{
    buzzerState = 2;
    buzzerStep = 0;
}
void bunyiBuzzerGagal()
{
    buzzerState = 3;
    buzzerStep = 0;
}

// --- FUNGSI UTILS LAINNYA ---

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

// --- PERBAIKAN DI SINI ---
String getTimestamp() {
    time_t now = time(nullptr);

    // 1000000000 adalah timestamp kira-kira tahun 2001.
    // Jika 'now' di bawah ini, berarti NTP belum sync (masih 1970).
    // Maka kirim string kosong agar Backend menggunakan NOW().
    if (now < 1000000000)
    {
        return "";
    }

    struct tm* timeinfo = localtime(&now);
    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", timeinfo);
    return String(timestamp);
}
// -------------------------

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

void setTemporaryMessage(String l1, String l2, int duration) {
    showLcd(l1, l2);
    uiOverride = true;
    uiTimer = millis() + duration;
}

#endif