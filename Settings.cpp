#include "Settings.h"

// Definisi variabel
const char* mDNS_hostname = "rfid-lab-pemrograman";
const char* ap_ssid = "pemrograman_SETUP_PORTAL";
const char* ap_password = "smkn1katapang2026";
const char* serverURL = "https://zreech-apiakses.hf.space/api/Aktivitas/tap";
const char* registerURL = "https://zreech-apiakses.hf.space/api/Scan/register";
const int ID_RUANGAN = 1;

const char* MASTER_CARDS[] = {
    "4C:E2:D0:38",
    "0C:C0:E5:37",  
    "07:BE:94:04" 
};

const int MASTER_CARD_COUNT = sizeof(MASTER_CARDS) / sizeof(MASTER_CARDS[0]);

bool isMasterCard(String uid) {
    for (int i = 0; i < MASTER_CARD_COUNT; i++) {
        if (uid.equalsIgnoreCase(MASTER_CARDS[i])) {
            return true;
        }
    }
    return false;
}

const String RESET_WIFI_PASSWORD = "satusampai8";
const int MAX_RESET_ATTEMPTS = 3;
const unsigned long RESET_COOLDOWN = 30000;