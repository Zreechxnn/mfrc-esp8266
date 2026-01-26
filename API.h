#ifndef API_H
#define API_H

#include "Globals.h"
#include "Utils.h"
#include "LCD.h"

void parseResponse(String& jsonResponse) {
    StaticJsonDocument<512> docResp;
    DeserializationError error = deserializeJson(docResp, jsonResponse);

    if (!error) {
        apiStatus = docResp["data"]["status"] | "-";
        apiMessage = docResp["data"]["message"] | "-";
        apiNamaKelas = docResp["data"]["namaKelas"] | "-";

        if (apiStatus.indexOf("SUKSES") >= 0) {
            String l1 = apiMessage;
            String l2 = apiNamaKelas;
            showLcd(l1, l2);
        } else {
            String errorMsg = apiMessage;
            showLcd("DITOLAK", errorMsg);
        }
    } else {
         showLcd("Response Error", "JSON Invalid");
    }
}

bool registerCard(const String& uid) {
    if (WiFi.status() != WL_CONNECTED) {
        showLcd("Gagal Register", "WiFi Offline");
        return false;
    }

    showLcd("DAFTAR KARTU", uid);

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(10000);

    HTTPClient http;

    if (!http.begin(client, registerURL)) {
        showLcd("Register Gagal", "URL Error");
        return false;
    }

    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<96> doc;
    doc["uid"] = uid;

    String json;
    serializeJson(doc, json);

    int httpCode = http.POST(json);

    if (httpCode > 0) {
        String res = http.getString();
        StaticJsonDocument<192> respDoc;
        DeserializationError error = deserializeJson(respDoc, res);

        if (!error) {
            bool success = respDoc["success"];
            if (success) {
                showLcd("REGISTER OK", "Kartu Terdaftar");
            } else {
                showLcd("REGISTER GAGAL", "Gagal Simpan");
            }
        } else {
            showLcd("JSON Error", "Bad Response");
        }
    } else {
        showLcd("Register Gagal", "HTTP: " + String(httpCode));
    }

    http.end();
    return (httpCode > 0);
}

int performRequest(WiFiClient& client, const String& uid, const String& timestamp) {
    HTTPClient http;
    if (!http.begin(client, serverURL)) return -1;

    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<192> docReq;
    docReq["uid"] = uid;
    docReq["idRuangan"] = ID_RUANGAN;
    docReq["timestamp"] = timestamp;

    String payload;
    serializeJson(docReq, payload);

    showLcd("MEMPROSES...", "Cek Database");

    int httpCode = http.POST(payload);

    if (httpCode > 0) {
        String res = http.getString();
        parseResponse(res);
    } else {
        showLcd("HTTP Error", "Code: " + String(httpCode));
    }

    http.end();
    return httpCode;
}

void sendData(const String& uid, const String& timestamp) {
    // Logika Offline Mode
    if (WiFi.status() != WL_CONNECTED) {
        if (offlineQueue.size() >= maxOfflineQueue) {
            offlineQueue.erase(offlineQueue.begin());
        }

        offlineQueue.push_back({uid, timestamp});
        showLcd("Data Disimpan", "Mode Offline");
        return;
    }

    int code = 0;

    if (String(serverURL).startsWith("https")) {
        WiFiClientSecure sClient;
        sClient.setInsecure();
        sClient.setTimeout(10000);
        code = performRequest(sClient, uid, timestamp);
    } else {
        WiFiClient nClient;
        nClient.setTimeout(10000);
        code = performRequest(nClient, uid, timestamp);
    }
}

void processOfflineQueue() {
    if (offlineQueue.empty() || WiFi.status() != WL_CONNECTED) return;

    const OfflineData& data = offlineQueue.front();

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    if (http.begin(client, serverURL)) {
        http.addHeader("Content-Type", "application/json");

        StaticJsonDocument<192> doc;
        doc["uid"] = data.uid;
        doc["idRuangan"] = ID_RUANGAN;
        doc["timestamp"] = data.timestamp;

        String payload;
        serializeJson(doc, payload);

        if (http.POST(payload) > 0) {
            offlineQueue.erase(offlineQueue.begin());
            showLcd("Sync Success", "Sisa: " + String(offlineQueue.size()));
        }
        http.end();
    }
}

#endif