#include "Settings.h"

// Definisi variabel (hanya di satu tempat)
const char* mDNS_hostname = "rfid-lab-PBO";
const char* ap_ssid = "PBO_SETUP_PORTAL";
const char* ap_password = "smkn1katapang2026";
const char* serverURL = "https://zreech-apiakses.hf.space/api/Aktivitas/tap";
const char* registerURL = "https://zreech-apiakses.hf.space/api/Scan/register";
const int ID_RUANGAN = 1;

const String MASTER_CARD_UID = "4C:E2:D0:38";
const String RESET_WIFI_PASSWORD = "satusampai8";
const int MAX_RESET_ATTEMPTS = 3;
const unsigned long RESET_COOLDOWN = 30000;