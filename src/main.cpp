#include <WiFi.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <DHT.h>
#include <main.h>
#include "time.h"

// === LED-Matrix Initalierung ===
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define DATA_PIN 23
#define CS_PIN   5
#define CLK_PIN  18

MD_Parola display = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
MD_MAX72XX pixels = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// === WLAN Konfiguration ===
// const char* ssid = "DEIN_WLAN_NAME";
// const char* password = "DEIN_WLAN_PASSWORT";

// === DHT22 Sensor ===
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// === NTP und Zeitzone ===
const char* tz = "CET-1CEST,M3.5.0/2,M10.5.0/3";  // automatische Sommerzeitumschaltung
// const char* ntpServer = "pool.ntp.org";  // NTP Server

// === Umschaltung zwischen Zeit und Temperaturanzeige ===
enum AnzeigePhase { PHASE_UHR, PHASE_TEMP };
AnzeigePhase phase = PHASE_UHR;
unsigned long letzteUmschaltung = 0;
const unsigned long anzeigedauer = 3000; // 3 Sekunden je Anzeige

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWLAN verbunden.");

  configTzTime(tz, ntpServer);  // automatisch Sommer-/Winterzeit

  display.begin();
  display.setIntensity(5);
  display.setTextAlignment(PA_CENTER);
  display.displayClear();

  pixels.begin();  // zweites Objekt ebenfalls initialisieren
  dht.begin();
}


void loop() {
  if (millis() - letzteUmschaltung > anzeigedauer) {
    letzteUmschaltung = millis();
    display.displayClear();

    if (phase == PHASE_UHR) {
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        char zeit[6];
        sprintf(zeit, "%02d %02d", timeinfo.tm_hour, timeinfo.tm_min);
        display.print(zeit);

        // manueller Doppelpunkt zwischen Stunde und Minute
        pixels.setPoint(15, 2, true);
        pixels.setPoint(15, 5, true);
      } else {
        display.print("NoNet");
      }
      phase = PHASE_TEMP;

    } else {
      float temp = dht.readTemperature();
      if (!isnan(temp)) {
        char temperatur[8];
        sprintf(temperatur, "%.0f\xDF""C", temp);  // °C → \xDF = Gradzeichen im Font
        display.print(temperatur);
      } else {
        display.print("Fehler");
      }
      phase = PHASE_UHR;
    }
  }
}