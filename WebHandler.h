#ifndef WEBHANDLER_H
#define WEBHANDLER_H

#include "Globals.h"
#include "Settings.h"
#include <WiFiManager.h>

void handleStatusJSON() {
    // OPTIMASI: StaticJsonDocument untuk response server lokal
    StaticJsonDocument<512> doc;

    doc["wifi_status"] = (WiFi.status() == WL_CONNECTED) ? "ONLINE" : "OFFLINE";
    doc["ip"] = WiFi.localIP().toString();
    doc["queue"] = offlineQueue.size();
    doc["uid"] = lastUID;
    doc["time"] = lastTime;
    doc["web_status"] = webStatus;

    doc["api_code"] = apiStatus;
    doc["api_msg"] = apiMessage;
    doc["api_nama"] = apiNamaKelas;
    doc["reset_attempts"] = resetAttempts;

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

bool verifyResetPassword(String password) {
    unsigned long currentMillis = millis();

    if (currentMillis - lastResetAttempt > RESET_COOLDOWN) {
        resetAttempts = 0;
    }

    if (resetAttempts >= MAX_RESET_ATTEMPTS) {
        return false;
    }

    if (password == RESET_WIFI_PASSWORD) {
        resetAttempts = 0;
        return true;
    } else {
        resetAttempts++;
        lastResetAttempt = currentMillis;
        return false;
    }
}

// HTML tetap sama, menggunakan PROGMEM sudah benar.
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang='id'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Monitor RFID Lab</title>
    <style>
        :root { --bg: #f8f9fa; --card: #ffffff; --text: #2d3436; --accent: #0984e3; --danger: #d63031; --success: #00b894; --warning: #fdcb6e; }
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: var(--bg); color: var(--text); padding: 20px; margin: 0; }
        .container { max-width: 500px; margin: 0 auto; }
        h1 { text-align: center; font-size: 1.4rem; color: #2d3436; margin-bottom: 25px; }
        .card { background: var(--card); padding: 20px; border-radius: 15px; box-shadow: 0 4px 6px rgba(0,0,0,0.05); margin-bottom: 20px; border-left: 5px solid var(--accent); }
        .row { display: flex; justify-content: space-between; margin-bottom: 12px; font-size: 0.95rem; border-bottom: 1px solid #f1f1f1; padding-bottom: 8px; }
        .row:last-child { border-bottom: none; }
        .label { color: #636e72; }
        .value { font-weight: 600; }
        .status-online { color: var(--success); }
        .status-offline { color: var(--danger); }
        .btn-reset { display: block; text-align: center; color: var(--danger); text-decoration: none; font-size: 0.8rem; margin-top: 30px; opacity: 0.7; }
        .btn-reset:hover { opacity: 1; }
        .modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.5); z-index: 1000; }
        .modal-content { background: white; margin: 15% auto; padding: 20px; width: 300px; border-radius: 10px; box-shadow: 0 4px 20px rgba(0,0,0,0.2); }
        .modal-title { margin-bottom: 15px; color: var(--danger); }
        .modal-input { width: 100%; padding: 10px; margin-bottom: 15px; border: 1px solid #ddd; border-radius: 5px; }
        .modal-buttons { display: flex; justify-content: flex-end; gap: 10px; }
        .modal-btn { padding: 8px 15px; border: none; border-radius: 5px; cursor: pointer; }
        .modal-btn.cancel { background: #f1f1f1; }
        .modal-btn.submit { background: var(--danger); color: white; }
        .warning { color: var(--warning); font-size: 0.8rem; margin-top: 5px; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>📡 Dashboard RFID Lab</h1>
        <div class='card'>
            <div class='row'><span class='label'>Status Alat</span> <span id='wifi-status' class='value'>Menghubungkan...</span></div>
            <div class='row'><span class='label'>IP Lokal</span> <span id='ip' class='value'>-</span></div>
            <div class='row'><span class='label'>Antrian Offline</span> <span id='queue' class='value'>0</span> Data</div>
            <div class='row'><span class='label'>Percobaan Reset</span> <span id='reset-attempts' class='value'>0</span> / 3</div>
        </div>
        <div class='card' style='border-left-color: #6c5ce7;'>
            <h3>Tap Terakhir</h3>
            <div class='row'><span class='label'>UID</span> <span id='uid' class='value'>-</span></div>
            <div class='row'><span class='label'>Waktu</span> <span id='time' class='value'>-</span></div>
            <div class='row'><span class='label'>Respon Alat</span> <span id='web_status' class='value'>Siap</span></div>
        </div>
        <div class='card' style='border-left-color: var(--success);'>
            <h3>Data Pengguna</h3>
            <div class='row'><span class='label'>Nama</span> <span id='api_nama' class='value'>-</span></div>
            <div class='row'><span class='label'>Pesan API</span> <span id='api_msg' class='value'>-</span></div>
        </div>
        <a href='#' class='btn-reset' id='reset-btn'>Reset Koneksi WiFi</a>
        <div class='warning'>* Membutuhkan password admin</div>
    </div>
    <div id='passwordModal' class='modal'>
        <div class='modal-content'>
            <div class='modal-title'>Reset WiFi</div>
            <p>Masukkan password admin untuk reset WiFi:</p>
            <input type='password' id='resetPassword' class='modal-input' placeholder='Password admin'>
            <div class='modal-buttons'>
                <button class='modal-btn cancel' id='cancelBtn'>Batal</button>
                <button class='modal-btn submit' id='submitBtn'>Reset</button>
            </div>
        </div>
    </div>
    <script>
        const resetBtn = document.getElementById('reset-btn');
        const modal = document.getElementById('passwordModal');
        const cancelBtn = document.getElementById('cancelBtn');
        const submitBtn = document.getElementById('submitBtn');
        const passwordInput = document.getElementById('resetPassword');
        resetBtn.addEventListener('click', (e) => { e.preventDefault(); modal.style.display = 'block'; passwordInput.value = ''; });
        cancelBtn.addEventListener('click', () => { modal.style.display = 'none'; });
        submitBtn.addEventListener('click', () => {
            const password = passwordInput.value;
            if (!password) { alert('Masukkan password!'); return; }
            fetch(`/resetwifi?password=${encodeURIComponent(password)}`)
                .then(response => response.text())
                .then(data => {
                    if (data.includes('Password salah')) { alert('Password salah!'); }
                    else if (data.includes('diblokir')) { alert('Reset diblokir. Tunggu 30 detik.'); }
                    else { alert('WiFi berhasil direset. Alat akan restart...'); modal.style.display = 'none'; }
                })
                .catch(error => { console.error('Error:', error); alert('Terjadi kesalahan'); });
        });
        window.addEventListener('click', (e) => { if (e.target === modal) { modal.style.display = 'none'; } });
        setInterval(function() {
            fetch('/status').then(res => res.json()).then(data => {
                const ws = document.getElementById('wifi-status');
                ws.innerText = data.wifi_status;
                ws.className = data.wifi_status === 'ONLINE' ? 'value status-online' : 'value status-offline';
                document.getElementById('ip').innerText = data.ip;
                document.getElementById('queue').innerText = data.queue;
                document.getElementById('uid').innerText = data.uid;
                document.getElementById('time').innerText = data.time;
                document.getElementById('web_status').innerText = data.web_status;
                document.getElementById('api_nama').innerText = data.api_nama;
                document.getElementById('api_msg').innerText = data.api_msg;
                document.getElementById('reset-attempts').innerText = data.reset_attempts;
                if (data.reset_attempts >= 3) {
                    document.getElementById('reset-btn').style.color = 'var(--warning)';
                    document.querySelector('.warning').innerText = '* Reset diblokir sementara';
                } else {
                    document.getElementById('reset-btn').style.color = 'var(--danger)';
                    document.querySelector('.warning').innerText = '* Membutuhkan password admin';
                }
            }).catch(err => console.error('Error polling:', err));
        }, 3000);
    </script>
</body>
</html>
)rawliteral";

void handleResetWiFi() {
    String password = server.arg("password");
    if (!verifyResetPassword(password)) {
        if (resetAttempts >= MAX_RESET_ATTEMPTS) {
            server.send(200, "text/html", "<h2>Reset diblokir! Tunggu 30 detik.</h2>");
        } else {
            server.send(200, "text/html", "<h2>Password salah! Percobaan: " + String(resetAttempts) + "/3</h2>");
        }
        return;
    }
    
    // Respons dulu sebelum reset agar browser tidak timeout
    server.send(200, "text/html", "<h2>WiFi berhasil direset! Restarting...</h2>");
    delay(500); 

    WiFiManager wm;
    wm.resetSettings();
    delay(1000);
    ESP.restart();
}

void handleRoot() {
    server.send(200, "text/html", INDEX_HTML);
}

#endif