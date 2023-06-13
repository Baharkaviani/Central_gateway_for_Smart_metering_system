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

// WiFi (Replace with your network credentials)
const char *ssid = "kaviani";  // Enter your WiFi name
const char *password = "hasti1318"; // Enter WiFi password
WiFiClient espClient;
PubSubClient client(espClient);

// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org";
const int mqtt_port = 1884;

// MQTT Topics
const char *gas_topic = "AUTSmartMeteringSystem/gas/ID1/consumption";
const char *water_topic = "AUTSmartMeteringSystem/water/ID1/consumption"; //AUTSmartMeteringSystem/Node1/WaterFlow1
const char *power_topic = "AUTSmartMeteringSystem/power/ID1/consumption";
const char *battery_topic = "AUTSmartMeteringSystem/battery/ID1/remainingPercentage";
const char *command_topic = "AUTSmartMeteringSystem/command/ID1/commandText";

// MQTT Authentication
const char *mqtt_username = "rw";
const char *mqtt_password = "readwrite";

// Required parameters to specify the data sending period
unsigned long lastGasMsgTime = 0;
unsigned long lastWaterMsgTime = 0;
unsigned long lastPowerMsgTime = 0;
unsigned long lastBatteryMsgTime = 0;

unsigned long elapsedTimeInSeconds = 0;

unsigned long updatingGasDataPeriod = 5000; // = 5 second
unsigned long updatingWaterDataPeriod = 10000; // = 10 second
unsigned long updatingPowerDataPeriod = 5000; // = 5 second
unsigned long updatingBatteryDataPeriod = 7000; // = 7 second

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
    client.subscribe(gas_topic);
    client.publish(gas_topic, "Gas consumption will be sent through this topic.");
    client.subscribe(water_topic);
    client.publish(water_topic, "Water consumption will be sent through this topic.");
    client.subscribe(power_topic);
    client.publish(power_topic, "Power consumption will be sent through this topic.");
    client.subscribe(battery_topic);
    client.publish(battery_topic, "Remaining battery percentage will be sent through this topic.");
    client.subscribe(command_topic);
    client.publish(command_topic, "Orders will be received through this topic.");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    elapsedTimeInSeconds = millis();

    // gas
    if (elapsedTimeInSeconds - lastGasMsgTime > updatingGasDataPeriod) {
        lastGasMsgTime = elapsedTimeInSeconds;
        send_gas_consumption(elapsedTimeInSeconds);
    }

    // water
    if (elapsedTimeInSeconds - lastWaterMsgTime > updatingWaterDataPeriod) {
        lastWaterMsgTime = elapsedTimeInSeconds;
        send_water_consumption(elapsedTimeInSeconds);
    }

    // power
    if (elapsedTimeInSeconds - lastPowerMsgTime > updatingPowerDataPeriod) {
        lastPowerMsgTime = elapsedTimeInSeconds;
        send_power_consumption(elapsedTimeInSeconds);
    }

    if (elapsedTimeInSeconds - lastBatteryMsgTime > updatingBatteryDataPeriod) {
        lastBatteryMsgTime = elapsedTimeInSeconds;
        send_battery_remaining(elapsedTimeInSeconds);
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
    Serial.print(gas_consumption);
    Serial.print(", time:");
    Serial.println(millis());

    // publish
    client.publish(gas_topic, gas_consumption);
}

void send_water_consumption(double water_n) {
    char water_consumption[10];

    // Convert the water consumption value to a char array
    dtostrf(water_n, 1, 2, water_consumption);

    Serial.print("water consumption: ");
    Serial.print(water_consumption);
    Serial.print(", time:");
    Serial.println(millis());

    // publish
    client.publish(water_topic, water_consumption);
}

void send_power_consumption(double power_n) {
    char power_consumption[10];

    // Convert the power consumption value to a char array
    dtostrf(power_n, 1, 2, power_consumption);

    Serial.print("power consumption: ");
    Serial.print(power_consumption);
    Serial.print(", time:");
    Serial.println(millis());

    // publish
    client.publish(power_topic, power_consumption);
}

void send_battery_remaining(double battery_n) {
    char battery_consumption[10];

    // Convert the battery remaining percentage value to a char array
    dtostrf(battery_n, 1, 2, battery_consumption);

    Serial.print("battery consumption: ");
    Serial.print(battery_consumption);
    Serial.print(", time:");
    Serial.println(millis());

    // publish
    client.publish(battery_topic, battery_consumption);
}

void receive_command_text() {
    // TODO
}