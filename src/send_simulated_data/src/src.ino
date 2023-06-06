#include <stdlib.h>
#include <ESP8266WiFi.h>

// #define MY_LED_PIN 1 // D0
int ledPin1 = 16; // D0
int ledPin2 = 5;  // D1
int ledPin3 = 4;  // D2

void setup() {
  Serial.begin(9600);
  Serial.println("akjsflaksfjlajksflakjsf..");
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
}

void loop() {
  digitalWrite(ledPin1, HIGH);
  delay(1000);
  digitalWrite(ledPin2, HIGH);
  delay(1000);
  digitalWrite(ledPin3, HIGH);
  delay(1000);
  digitalWrite(ledPin1, LOW);
  delay(1000);
  digitalWrite(ledPin2, LOW);
  delay(1000);
  digitalWrite(ledPin3, LOW);
  delay(1000);
}