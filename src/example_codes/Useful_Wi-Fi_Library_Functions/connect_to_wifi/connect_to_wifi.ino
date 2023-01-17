/*
*******************************************************************************
* Writer: Bahar Kaviani
* Describe:
    This sketch demonstrates how to connect to a WiFi network.
    Complete details at https://RandomNerdTutorials.com/esp32-useful-wi-fi-functions-arduino/
* Date: 2023/01/11
*******************************************************************************
*/

#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "Shenasa";
const char* password = "Shenasa@VC";

void setup() {
  Serial.begin(115200);
  initWiFi();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
}

void loop() {
  // put your main code here, to run repeatedly:
}

void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println('.');
        delay(1000);
    }
    Serial.println(WiFi.localIP());
    Serial.println("The board is connected to the wifi");
}