/*
   Olimex ESP32-C5-DevKit-LiPo WiFi Band Scanner
   -------------------------------------------------------------
    • LED1 (GPIO27, active LOW) shows:
        - 2.4 GHz band → Slow blink (1 Hz)
        - 5 GHz band → Double blink pattern
        - Scanning → Fast blink (4 Hz)
    • Button BOOT1 (GPIO28, active LOW, pulled-up) toggles between bands
    • Auto Wi-Fi scan every 10 s

  Board Manager Settings if you use USB-UART1:

    - Board: ESP32C5 Dev Module
    - PSRAM: Enabled
    - USB CDC on Boot: Disabled
    - The COM port associated with the board

   How to use:
    - Open Serial Monitor from Tools -> Serial Monitor
    - Wait until all WIFI networks are printed
    - Press the BOOT1 button -> switches to 5GHz band and scans again
    - Press the BOOT1 button -> switches back to 2.4Ghz band and scans again
 */

#include "WiFi.h"

#define USER_LED 27
#define USER_BTN 28

bool bandIs24GHz = true;        // start with 2.4 GHz
volatile bool buttonPressed = false;
bool scanning = false;
unsigned long lastBlink = 0;
unsigned long blinkInterval = 250;
unsigned long lastScan = 0;
const unsigned long scanInterval = 10000;

// --- ISR for button ---
void IRAM_ATTR handleButton() {
  static unsigned long lastPress = 0;
  unsigned long now = millis();
  if (now - lastPress > 300) {  // debounce
    buttonPressed = true;
    lastPress = now;
  }
}

// --- Wi-Fi Scan ---
void ScanWiFi() {
  scanning = true;
  Serial.printf("\n--- Scanning %s networks ---\n", bandIs24GHz ? "2.4 GHz" : "5 GHz");
  blinkInterval = 125;  // fast blink

  // Set explicit band mode (prevents AUTO error)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,4,2)
  if (bandIs24GHz)
    WiFi.setBandMode(WIFI_BAND_MODE_2G_ONLY);
  else
    WiFi.setBandMode(WIFI_BAND_MODE_5G_ONLY);
#endif

  int n = WiFi.scanNetworks();
  Serial.println("Scan done");

  if (n <= 0) {
    Serial.println("No networks found");
  } else {
    Serial.printf("%d networks found\n", n);
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    for (int i = 0; i < n; ++i) {
      Serial.printf("%2d | %-32.32s | %4ld | %2ld | ",
                    i + 1, WiFi.SSID(i).c_str(),
                    WiFi.RSSI(i), WiFi.channel(i));
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:            Serial.print("open"); break;
        case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
        case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
        case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
        case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
        case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
        case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
        case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
        case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
        default:                        Serial.print("unknown");
      }
      Serial.println();
      delay(5);
    }
  }

  WiFi.scanDelete();
  Serial.println("-------------------------------------");

  scanning = false;
  lastScan = millis();
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nOlimex ESP32-C5-DevKit-LiPo Wi-Fi Scanner");
  Serial.println("-----------------------------------------");

  pinMode(USER_LED, OUTPUT);
  digitalWrite(USER_LED, HIGH);  // LED off (active LOW)
  pinMode(USER_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(USER_BTN), handleButton, FALLING);

  // Fix: prevent "AUTO" mode warning
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,4,2)
  WiFi.setBandMode(WIFI_BAND_MODE_2G_ONLY);
#endif

  Serial.println("Starting initial scan...");
  ScanWiFi();
}

// --- LED behavior ---
void updateLedPattern() {
  static bool ledOn = false;
  static int blinkCount = 0;
  unsigned long now = millis();

  if (scanning) {
    blinkInterval = 125;
  } else if (bandIs24GHz) {
    blinkInterval = 500;  // normal blink
  } else {
    blinkInterval = 250;  // used for double blink logic
  }

  if (now - lastBlink >= blinkInterval) {
    lastBlink = now;

    if (bandIs24GHz || scanning) {
      ledOn = !ledOn;
      digitalWrite(USER_LED, ledOn ? LOW : HIGH);
    } else {
      // Double blink pattern for 5 GHz
      if (blinkCount == 0 || blinkCount == 2) {
        digitalWrite(USER_LED, LOW);  // on
      } else {
        digitalWrite(USER_LED, HIGH); // off
      }
      blinkCount = (blinkCount + 1) % 6;
    }
  }
}

// --- Loop ---
void loop() {
  updateLedPattern();

  if (buttonPressed) {
    buttonPressed = false;
    bandIs24GHz = !bandIs24GHz;
    Serial.printf("\nButton pressed → Switching to %s mode\n",
                  bandIs24GHz ? "2.4 GHz" : "5 GHz");
    ScanWiFi();
  }

  if (!scanning && (millis() - lastScan >= scanInterval)) {
    ScanWiFi();
  }
}
