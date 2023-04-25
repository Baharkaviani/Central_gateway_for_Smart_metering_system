#include <ESP8266WiFi.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//const String ssid = "Zara's iPhone";
//const String password = "amberbrown";
const String ssid = "Emili";
const String password = "amber1234";

const String mqtt_broker = "192.168.1.4";
const int mqtt_port = 1883;

const long period = 120000;
const long logperiod = 60000;
long timeOfPreviousPing;
long timeOfPreviousLog;

WiFiClient espClient;
MqttClient mqtt_client(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const int LED_PIN1 = 16; //D0
const int LED_PIN2 = 5; //D1
const int LED_PIN3 = 4; //D2
const int LED_PIN4 = 0; //D3
const int LDR_PIN = A0;

//////////////////////////////////////////////////////////setup//////////////////////////////////////////////////////////
void setup() {
    Serial.begin(9600);
    Serial.println("akjsflaksfjlajksflakjsf..");
    
    timeOfPreviousPing = millis();
    timeOfPreviousLog = millis();

    pinMode(LED_PIN1, OUTPUT);
    pinMode(LED_PIN2, OUTPUT);
    pinMode(LED_PIN3, OUTPUT);
    pinMode(LED_PIN4, OUTPUT);
    digitalWrite(LED_PIN1, LOW);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
    digitalWrite(LED_PIN4, LOW);
    
    WiFi.begin(ssid, password);

    Serial.println("Connecting ...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println(".");
    }
    
    Serial.println("Connected to the WiFi network");

    while (!mqtt_client.connect(mqtt_broker.c_str(), mqtt_port)) {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqtt_client.connectError());
    }

    Serial.println("MQTT connection established");
    
    mqtt_client.subscribe("I1820/main/configuration/request");
    mqtt_client.onMessage(notif_mia_func);
}
//////////////////////////////////////////////////////////loop//////////////////////////////////////////////////////////
void loop() {
    mqtt_client.poll();
    
    if (millis() - timeOfPreviousPing >= period) {
      ping_mia_func(); 
      timeOfPreviousPing = millis();
    }

    if (millis() - timeOfPreviousLog >= logperiod) {
      log_mia_func();
      timeOfPreviousLog = millis();
    }

    delay(200);
}
/////////////////////////////////////////////////////ping_mia_func////////////////////////////////////////////////////////
void ping_mia_func() {
  String ping = "{\"id\": \"MiA_BZ_Agent\",\"things\": [{ \"id\": \"LDR\", \"type\": \"light\" },{ \"id\": \"lamp_1\", \"type\": \"lamp\" },{ \"id\": \"lamp_2\", \"type\": \"lamp\" },{ \"id\": \"lamp_3\", \"type\": \"lamp\" },{ \"id\": \"lamp_4\", \"type\": \"lamp\" }],\"actions\": []}";

  Serial.println("pinging Mia...");

  mqtt_client.beginMessage("I1820/main/agent/ping");
  mqtt_client.print(ping);
  mqtt_client.endMessage();

  Serial.println("pinging complete");
}
/////////////////////////////////////////////////////log_mia_func////////////////////////////////////////////////////////
void log_mia_func() {
  Serial.print("TimeStamp: ");
  Serial.print(timeClient.getFormattedTime());
  Serial.println("");
  
  int ldr_value = analogRead(LDR_PIN);
  int mapped_value = map(ldr_value, 0, 1024, 100, 0);
  String ldr_message = "{\"timestamp\":" + String(timeClient.getEpochTime()) + ",\"type\":\"light\",\"device\":\"LDR\",\"states\":[{\"name\":\"light\",\"value\":" + mapped_value + "}],\"agent\":\"MiA_BZ_Agent\"}";
  Serial.println(ldr_message);
  
  mqtt_client.beginMessage("I1820/main/log/send");
  mqtt_client.print(ldr_message);
  mqtt_client.endMessage();
}
/////////////////////////////////////////////////////notif_mia_func////////////////////////////////////////////////////////
void notif_mia_func(int messageLength) {
  Serial.print("Topic: ");
  Serial.println(mqtt_client.messageTopic());
  Serial.print("Length: ");
  Serial.println(messageLength);

  String jsonString = "";

  while (mqtt_client.available()) {
    jsonString += (char) mqtt_client.read();
  }
  
  Serial.println("Message JSON: ");
  Serial.println(jsonString);


  StaticJsonDocument <200> json;
  deserializeJson(json, jsonString);

  if ((json["agent"] == "MiA_BZ_Agent") && (json["settings"][0]["name"] == "on")) {
    if (json["device"] == "lamp_1"){
//      Serial.print("digitalWrite: lamp_1");
      digitalWrite(LED_PIN1, json["settings"][0]["value"]);
    }
    else if(json["device"] == "lamp_2"){
  //    Serial.print("digitalWrite: lamp_2");
      digitalWrite(LED_PIN2, json["settings"][0]["value"]);
    }
    else if(json["device"] == "lamp_3"){
    //  Serial.print("digitalWrite: lamp_3");
      digitalWrite(LED_PIN3, json["settings"][0]["value"]);
    }
    else if(json["device"] == "lamp_4"){
      //Serial.print("digitalWrite: lamp_4");
      digitalWrite(LED_PIN4, json["settings"][0]["value"]);
    }
  }
}
