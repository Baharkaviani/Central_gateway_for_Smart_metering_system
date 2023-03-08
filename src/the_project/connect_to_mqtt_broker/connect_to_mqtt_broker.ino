/*
*******************************************************************************
* Writer: Bahar Kaviani
* Describe:
    This sketch demonstrates how to connect to a mqtt broker.
    Before connecting to broker you have to test wifi connection.
* Date: 2023/02/14
*******************************************************************************
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>

// Replace with your network credentials
const char* ssid = "*******";
const char* password = "*******";

// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org";
const int mqtt_port = 1884;

// MQTT Topics
const char *gas_topic = "AUTSmartMeteringSystem/gas/ID1/consumption";
const char *water_topic = "AUTSmartMeteringSystem/water/ID1/consumption";
const char *power_topic = "AUTSmartMeteringSystem/power/ID1/consumption";
const char *battery_topic = "AUTSmartMeteringSystem/battery/ID1/remainingPercentage";

// MQTT Authentication
const char *mqtt_username = "rw";
const char *mqtt_password = "readwrite";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
long updatingDataPeriod = 10000;

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);

    // connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to the WiFi network");

    // connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    reconnect();

    // publish and subscribe
    client.publish(gas_topic, "gas gas gas");
    client.subscribe(gas_topic);
    client.publish(water_topic, "water water water");
    client.subscribe(water_topic);
    client.publish(power_topic, "power power power");
    client.subscribe(power_topic);
    client.publish(battery_topic, "battery battery battery");
    client.subscribe(battery_topic);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > updatingDataPeriod) {
        lastMsg = now;

        send_gas_consumption(0);
        send_water_consumption(1);
        send_power_consumption(2);
        send_battery_remaining(3);
    }
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...\n");

        String client_id = "aut_esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());

        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        // if (client.connect(client_id.c_str())) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            Serial.println(" try again in 2 seconds");
            // Wait 2 seconds before retrying
            delay(2000);
        }
    }
}

void send_gas_consumption(double gas_n) {
    char gas_consumption[10];

    // Convert the gas consumption value to a char array
    dtostrf(gas_n, 1, 2, gas_consumption);
    Serial.print("gas consumption: ");
    Serial.println(gas_consumption);
    client.publish(gas_topic, gas_consumption);
}

void send_water_consumption(double water_n) {
    char water_consumption[10];

    // Convert the water consumption value to a char array
    dtostrf(water_n, 1, 2, water_consumption);
    Serial.print("water consumption: ");
    Serial.println(water_consumption);
    client.publish(water_topic, water_consumption);
}

void send_power_consumption(double power_n) {
    char power_consumption[10];

    // Convert the power consumption value to a char array
    dtostrf(power_n, 1, 2, power_consumption);
    Serial.print("power consumption: ");
    Serial.println(power_consumption);
    client.publish(power_topic, power_consumption);
}

void send_battery_remaining(double battery_n) {
    char battery_consumption[10];

    // Convert the battery remaining percentage value to a char array
    dtostrf(battery_n, 1, 2, battery_consumption);
    Serial.print("battery consumption: ");
    Serial.println(battery_consumption);
    client.publish(battery_topic, battery_consumption);
}