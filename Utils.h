#ifndef UTILS_H
#define UTILS_H

#include "Globals.h"

// Definisi Variabel Buzzer (Definisi fisik ada di .ino, ini deklarasi untuk Utils)
int buzzerState = 0;
int buzzerStep = 0;
unsigned long buzzerTimer = 0;
unsigned long uiTimer = 0;
bool uiOverride = false;

// --- LOGIKA BUZZER NON-BLOCKING ---
void handleBuzzerLoop()
{
    unsigned long now = millis();
    if (buzzerState == 0)
        return;

    // Beep Pendek (Normal)
    if (buzzerState == 1)
    {
        if (buzzerStep == 0)
        {
            digitalWrite(BUZZER_PIN, HIGH);
            buzzerTimer = now;
            buzzerStep = 1;
        }
        else if (buzzerStep == 1 && now - buzzerTimer > 100)
        {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerState = 0; // Selesai
            buzzerStep = 0;
        }
    }
    // Beep Sukses (2x pendek cepat)
    else if (buzzerState == 2)
    {
        if (buzzerStep == 0)
        {
            digitalWrite(BUZZER_PIN, HIGH);
            buzzerTimer = now;
            buzzerStep = 1;
        }
        else if (buzzerStep == 1 && now - buzzerTimer > 80)
        {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerTimer = now;
            buzzerStep = 2;
        }
        else if (buzzerStep == 2 && now - buzzerTimer > 50)
        {
            digitalWrite(BUZZER_PIN, HIGH);
            buzzerTimer = now;
            buzzerStep = 3;
        }
        else if (buzzerStep == 3 && now - buzzerTimer > 80)
        {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerState = 0;
            buzzerStep = 0;
        }
    }
    // Beep Gagal (Panjang)
    else if (buzzerState == 3)
    {
        if (buzzerStep == 0)
        {
            digitalWrite(BUZZER_PIN, HIGH);
            buzzerTimer = now;
            buzzerStep = 1;
        }
        else if (buzzerStep == 1 && now - buzzerTimer > 800)
        { // 800ms
            digitalWrite(BUZZER_PIN, LOW);
            buzzerState = 0;
            buzzerStep = 0;
        }
    }
}

// Fungsi Pemicu (Trigger) - Tidak ada delay di sini!
void bunyiBuzzer()
{
    buzzerState = 1;
    buzzerStep = 0;
}
void bunyiBuzzerSukses() {
    buzzerState = 2;
    buzzerStep = 0;
}
void bunyiBuzzerGagal() {
    buzzerState = 3;
    buzzerStep = 0;
}
// ------------------------------------

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

// Helper untuk set UI sementara (pengganti delay di loop)
void setTemporaryMessage(String l1, String l2, int duration)
{
    showLcd(l1, l2);
    uiOverride = true;
    uiTimer = millis() + duration;
}

#endif