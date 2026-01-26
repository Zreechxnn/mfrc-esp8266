#ifndef WEBHANDLER_H
#define WEBHANDLER_H

#include "Globals.h"
#include "Settings.h"
#include <WiFiManager.h>

bool verifyResetPassword(const String& password) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastResetAttempt > RESET_COOLDOWN) {
        resetAttempts = 0;
    }
    if (resetAttempts >= MAX_RESET_ATTEMPTS) return false;

    if (password == RESET_WIFI_PASSWORD) {
        resetAttempts = 0;
        return true;
    } else {
        resetAttempts++;
        lastResetAttempt = currentMillis;
        return false;
    }
}

// HTML Minimalis - Hanya untuk Reset
const char MINIMAL_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>RFID Admin</title>
    <style>
        body { font-family: sans-serif; padding: 20px; text-align: center; }
        input { padding: 10px; margin: 10px 0; width: 80%; max-width: 300px; }
        button { padding: 10px 20px; background: #d63031; color: white; border: none; border-radius: 5px; cursor: pointer; }
    </style>
</head>
<body>
    <h2>Sistem RFID Online</h2>
    <p>Masukkan password admin untuk reset WiFi:</p>
    <form action='/resetwifi' method='POST'>
        <input type='password' name='password' placeholder='Password Admin' required><br>
        <button type='submit'>Reset WiFi</button>
    </form>
</body>
</html>
)rawliteral";

void handleResetWiFi() {
    String password = server.arg("password");
    if (!verifyResetPassword(password)) {
        if (resetAttempts >= MAX_RESET_ATTEMPTS) {
            server.send(200, "text/html", "<h2>Reset diblokir! Tunggu 30 detik.</h2><a href='/'>Kembali</a>");
        } else {
            server.send(200, "text/html", "<h2>Password salah!</h2><a href='/'>Coba Lagi</a>");
        }
        return;
    }

    server.send(200, "text/html", "<h2>WiFi direset! Alat akan restart...</h2>");
    delay(500);

    WiFiManager wm;
    wm.resetSettings();
    delay(1000);
    ESP.restart();
}

void handleRoot() {
    server.send(200, "text/html", MINIMAL_HTML);
}

#endif