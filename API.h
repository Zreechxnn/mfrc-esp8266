#ifndef API_H
#define API_H

#include "Globals.h"
#include "Utils.h"
#include "LCD.h"

// Gunakan StaticJsonDocument agar tidak perlu new/delete manual
// Ukuran disesuaikan dengan kebutuhan payload

bool registerCard(String uid) {
    if (WiFi.status() != WL_CONNECTED) {
        showLcd("Gagal Register", "WiFi Offline");
        return false;
    }

    showLcd("DAFTAR KARTU", uid);

    WiFiClientSecure client;
    client.setInsecure(); // Bypass SSL check untuk HuggingFace
    client.setTimeout(10000); // Timeout 10 detik

    HTTPClient http;

    if (!http.begin(client, registerURL)) {
        showLcd("Register Gagal", "URL Error");
        return false;
    }

    http.addHeader("Content-Type", "application/json");

    // OPTIMASI: Gunakan StaticJsonDocument (tanpa pointer/new)
    StaticJsonDocument<128> doc;
    doc["uid"] = uid;

    String json;
    serializeJson(doc, json);

    int httpCode = http.POST(json);

    if (httpCode > 0) {
        String res = http.getString();
        // Buffer response sedikit lebih besar
        StaticJsonDocument<512> respDoc;
        DeserializationError error = deserializeJson(respDoc, res);

        if (!error) {
            bool success = respDoc["success"];
            if (success) {
                showLcd("REGISTER OK", "Kartu Terdaftar");
                bunyiBuzzerSukses();
            } else {
                showLcd("REGISTER GAGAL", "Gagal Simpan");
                bunyiBuzzerGagal();
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

int performRequest(WiFiClient& client, String uid, String timestamp) {
    HTTPClient http;
    if (!http.begin(client, serverURL)) return -1;

    http.addHeader("Content-Type", "application/json");

    // Request kecil, pakai Static aman
    StaticJsonDocument<256> docReq;
    docReq["uid"] = uid;
    docReq["idRuangan"] = ID_RUANGAN;
    docReq["timestamp"] = timestamp;

    String payload;
    serializeJson(docReq, payload);

    showLcd("MEMPROSES...", "Cek Database");

    int httpCode = http.POST(payload);

    if (httpCode > 0) {
        String res = http.getString();

        // PERBAIKAN: Gunakan DynamicJsonDocument untuk buffer besar (>512 bytes)
        // Agar tidak menjebol Stack Memory ESP8266
        DynamicJsonDocument docResp(1024); 
        DeserializationError error = deserializeJson(docResp, res);

        if (!error) {
            apiStatus = docResp["data"]["status"] | "-";
            apiMessage = docResp["data"]["message"] | "-";
            apiNamaKelas = docResp["data"]["namaKelas"] | "-";

            if (apiStatus.indexOf("SUKSES") >= 0) {
                String l1 = apiMessage;
                if (l1.length() > 16) l1 = l1.substring(0, 16);

                String l2 = apiNamaKelas;
                if (l2.length() > 16) l2 = l2.substring(0, 16);

                showLcd(l1, l2);
                bunyiBuzzerSukses();
            } else {
                String errorMsg = apiMessage;
                if (errorMsg.length() > 16) errorMsg = errorMsg.substring(0, 16);
                showLcd("DITOLAK", errorMsg);
                bunyiBuzzerGagal();
            }
        } else {
             showLcd("Response Error", "JSON Invalid");
        }
    } else {
        showLcd("HTTP Error", "Code: " + String(httpCode));
    }

    http.end();
    return httpCode;
}

void sendData(String uid, String timestamp) {
    if (WiFi.status() != WL_CONNECTED) {
        OfflineData* data = new OfflineData{uid, timestamp};

        // Cegah memory leak di antrian offline
        if (offlineQueue.size() >= 50) {
            delete offlineQueue.front();
            offlineQueue.pop_front();
        }

        offlineQueue.push_back(data);
        showLcd("Data Disimpan", "Mode Offline");
        return;
    }

    webStatus = "Mengirim...";
    int code = 0;

    // Deteksi HTTPS vs HTTP
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

    webStatus = (code > 0) ? "Sukses: " + String(code) : "Err: " + String(code);
}

void processOfflineQueue() {
    if (offlineQueue.empty() || WiFi.status() != WL_CONNECTED) return;

    // Ambil data tapi JANGAN pop dulu sebelum sukses kirim
    OfflineData* data = offlineQueue.front();

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    if (http.begin(client, serverURL)) {
        http.addHeader("Content-Type", "application/json");

        StaticJsonDocument<256> doc;
        doc["uid"] = data->uid;
        doc["idRuangan"] = ID_RUANGAN;
        doc["timestamp"] = data->timestamp;

        String payload;
        serializeJson(doc, payload);

        if (http.POST(payload) > 0) {
            // Sukses kirim, baru hapus dari memori
            delete data;
            offlineQueue.pop_front();
            showLcd("Sync Success", "Sisa: " + String(offlineQueue.size()));
        }
        http.end();
    }
}

#endif