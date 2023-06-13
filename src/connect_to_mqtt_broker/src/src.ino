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
unsigned long last_gas_msg_time = 0;
unsigned long last_water_msg_time = 5000;
unsigned long last_power_msg_time = 4000;
unsigned long last_battery_msg_time = 6000;

unsigned long elapsed_time_in_seconds = 0;

unsigned long updating_gas_data_period = 10000; // = 10 second
unsigned long updating_water_data_period = 10000; // = 10 second
unsigned long updating_power_data_period = 10000; // = 10 second
unsigned long updating_battery_data_period = 10000; // = 10 second

// Measured data to send
char gas_consumption[10][50];
char water_consumption[10];
char power_consumption[10];
char battery_consumption[10];
bool gas_subscribe[10] = {true};

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

    elapsed_time_in_seconds = millis();

    // gas
    if (elapsed_time_in_seconds - last_gas_msg_time > updating_gas_data_period) {
        last_gas_msg_time = elapsed_time_in_seconds;
        send_gas_consumption(elapsed_time_in_seconds);
    }

    // water
    if (elapsed_time_in_seconds - last_water_msg_time > updating_water_data_period) {
        last_water_msg_time = elapsed_time_in_seconds;
        send_water_consumption(elapsed_time_in_seconds);
    }

    // // power
    // if (elapsed_time_in_seconds - last_power_msg_time > updating_power_data_period) {
    //     last_power_msg_time = elapsed_time_in_seconds;
    //     send_power_consumption(elapsed_time_in_seconds);
    // }

    // // battery
    // if (elapsed_time_in_seconds - last_battery_msg_time > updating_battery_data_period) {
    //     last_battery_msg_time = elapsed_time_in_seconds;
    //     send_battery_remaining(elapsed_time_in_seconds);
    // }
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
    // find the index that the new data must be store in
    int unsendDataIndex = 0;
    for (int i = 0; i < 10; i++) {
        if (gas_subscribe[i]) {
            unsendDataIndex = i;
            break;
        }
    }

    // Convert the gas consumption value to a char array
    dtostrf(gas_n, 1, 2, gas_consumption[unsendDataIndex]);

    // print all gas_consumption datas
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 50; j++) {
            if (gas_consumption[i][j] == '\0') {
                Serial.println();
                break;
            }
            else {
                Serial.print(gas_consumption[i][j]);
            }
        }
    }
    
    // Create JSON object as a string
    String json_str = "{\"gas_consumption\": [";
    for (int i = 0; i <= unsendDataIndex; i++) {
        json_str += "[";
        for (int j = 0; j < 50; j++) {
            if (gas_consumption[i][j] != '\0') {
                json_str += gas_consumption[i][j];
            }
            else {
                break;
            }
            // json_str += ",";
        }
        json_str += "]";
        if (i != unsendDataIndex) {
            json_str += ",";
        }
        gas_subscribe[i] = true;
    }
    json_str += "]}";

    Serial.println(json_str);
    Serial.print("time:");
    Serial.println(millis());

    // Call getBytes() on that string to get the byte array to use as your payload in the message
    char payload[json_str.length()];
    int i;
    for (i = 0; i < json_str.length(); i++) {
        payload[i] = json_str[i];
    }
    payload[i] = '\0';
    // json_str.getBytes(payload, json_str.length());

    // publish
    // gas_subscribe = client.subscribe(gas_topic);
    client.publish(gas_topic, payload);
}

void send_water_consumption(double water_n) {
    // Convert the water consumption value to a char array
    dtostrf(water_n, 1, 2, water_consumption);

    Serial.print("water consumption: ");
    Serial.print(water_consumption);
    Serial.print(", time:");
    Serial.println(millis());

    // publish
    client.publish(water_topic, water_consumption);
    Serial.println("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww");
}

void send_power_consumption(double power_n) {
    // Convert the power consumption value to a char array
    dtostrf(power_n, 1, 2, power_consumption);

    Serial.print("power consumption: ");
    Serial.print(power_consumption);
    Serial.print(", time:");
    Serial.println(millis());

    // publish
    client.publish(power_topic, power_consumption);
    Serial.println("pppppppppppppppppppppppppppppppppppppppp");
}

void send_battery_remaining(double battery_n) {
    // Convert the battery remaining percentage value to a char array
    dtostrf(battery_n, 1, 2, battery_consumption);

    Serial.print("battery consumption: ");
    Serial.print(battery_consumption);
    Serial.print(", time:");
    Serial.println(millis());

    // publish
    client.publish(battery_topic, battery_consumption);
    Serial.println("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
}

void receive_command_text() {
    // TODO
}