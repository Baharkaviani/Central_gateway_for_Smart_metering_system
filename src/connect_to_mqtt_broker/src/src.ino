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
#include <M5Stack.h>

// WiFi (Replace with your network credentials)
const char *ssid = "Warning";  // Enter your WiFi name
const char *password = "zilc8261"; // Enter WiFi password
WiFiClient espClient;
PubSubClient client(espClient);

// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org";
const int mqtt_port = 1884;
const char *mqtt_username = "rw";
const char *mqtt_password = "readwrite";

struct TopicInfo {
    const char *topic;
    const char *payload;
};

TopicInfo topics[] = {
    {"AUTSmartMeteringSystem/gas/ID1/consumption", "Gas consumption will be sent through this topic."},
    {"AUTSmartMeteringSystem/water/ID1/consumption", "Water consumption will be sent through this topic."},
    {"AUTSmartMeteringSystem/power/ID1/consumption", "Power consumption will be sent through this topic."},
    {"AUTSmartMeteringSystem/battery/ID1/remainingPercentage", "Remaining battery percentage will be sent through this topic."},
    {"AUTSmartMeteringSystem/command/ID1/commandText", "Orders will be received through this topic."}
};

enum DataType {
    GAS,
    WATER,
    POWER,
    BATTERY
};

// Input pins
const int gas_pin_num = 35;
const int water_pin_num = 36;

// Required parameters to specify the data sending period
unsigned long last_gas_msg_time = 0;
unsigned long last_water_msg_time = 0;
unsigned long last_power_msg_time = 0;
unsigned long last_battery_msg_time = 0;
const int updating_data_period = 20000;

// Measured data to send
#define DATA_TYPES 4
#define BUFFER_SIZE 4
#define DATA_SIZE 50
char consumption_data[DATA_TYPES][BUFFER_SIZE][DATA_SIZE] = {{"0.0", "0.0", "0.0", "0.0"},
                                                             {"0.0", "0.0", "0.0", "0.0"},
                                                             {"0.0", "0.0", "0.0", "0.0"},
                                                             {"0.0", "0.0", "0.0", "0.0"}};
int unsent_index[DATA_TYPES] = {0};

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);

    // prepare board
    M5.begin();
    M5.Power.begin();
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Welcome :)");
    
    pinMode (gas_pin_num, INPUT);
    pinMode (water_pin_num, INPUT);

    // connecting to a WiFi network
    WiFiInit();

    // connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    reconnectBroker();

    // publish and subscribe
    for (TopicInfo topic : topics) {
        client.subscribe(topic.topic);
        client.publish(topic.topic, topic.payload);
    }
}

void loop() {
    if (!client.connected()) {
        reconnectBroker();
        for (TopicInfo topic : topics) {
            client.subscribe(topic.topic);
        }
    }
    client.loop();

    unsigned long elapsed_time_in_seconds = millis();

    // gas
    if (elapsed_time_in_seconds - last_gas_msg_time > updating_data_period) {
        last_gas_msg_time = elapsed_time_in_seconds;
        send_data(gas_pin_num, GAS);
    }

    // water
    if (elapsed_time_in_seconds - last_water_msg_time > updating_data_period) {
        last_water_msg_time = elapsed_time_in_seconds;
        send_data(water_pin_num, WATER);
    }

    // power
    if (elapsed_time_in_seconds - last_power_msg_time > updating_data_period) {
        last_power_msg_time = elapsed_time_in_seconds;
        double power_n = 42.0; // Replace with your actual power value
        send_data(power_n, POWER);
    }

    // battery
    if (elapsed_time_in_seconds - last_battery_msg_time > updating_data_period) {
        last_battery_msg_time = elapsed_time_in_seconds;
        send_battery_remaining();
    }
}

void WiFiInit() {
    // delete old config
    WiFi.disconnect(true);

    delay(1000);

    // handle different Wi-Fi events
    WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println("Connected to AP successfully!");
    M5.Lcd.println("Connected to AP successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println("Disconnected from WiFi access point.");
    Serial.print("WiFi lost connection. Reason: ");
    Serial.println(info.wifi_sta_disconnected.reason);
    Serial.println("Trying to Reconnect...");
    WiFi.begin(ssid, password);
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void reconnectBroker() {
    // Loop until we're reconnected to mqtt broker
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...\n");

        String client_id = "aut_esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());

        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
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

void send_data(int pin, DataType type) {
    uint32_t pin_data = digitalRead(pin);
    dtostrf(pin_data, 1, 2, consumption_data[type][unsent_index[type]]);
    send_topic_data(type);
}

void send_data(double value, DataType type) {
    dtostrf(value, 1, 2, consumption_data[type][unsent_index[type]]);
    send_topic_data(type);
}

void send_battery_remaining() {
    M5.Lcd.clearDisplay();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("Battery is Charging: ");
    M5.Lcd.println(M5.Power.isCharging());

    uint32_t battery_level = M5.Power.getBatteryLevel();
    M5.Lcd.print("Battery Level: ");
    M5.Lcd.println(battery_level);

    dtostrf(battery_level, 1, 0, consumption_data[BATTERY][unsent_index[BATTERY]]);
    send_topic_data(BATTERY);
}

void send_topic_data(DataType type) {
    String json_str = "{\"";
    json_str += get_topic_name(type);
    json_str += "\": [";

    for (int i = 0; i < BUFFER_SIZE; i++) {
        json_str += consumption_data[type][i];
        if (i < BUFFER_SIZE - 1) {
            json_str += ",";
        }
    }
    json_str = json_str + "], \"unsent_index\": " + unsent_index[type] + "}";

    Serial.println(json_str);
    Serial.print("time:");
    Serial.println(millis());

    char payload[json_str.length() + 1];
    snprintf(payload, sizeof(payload), "%s", json_str.c_str());

    client.publish(topics[type].topic, payload);

    if (unsent_index[type] < BUFFER_SIZE - 1) {
        unsent_index[type]++;
    } else {
        unsent_index[type] = 0;
    }
}

const char* get_topic_name(DataType type) {
    switch (type) {
        case GAS:
            // return topics[GAS].topic;
            return "GAS";
        case WATER:
            // return topics[WATER].topic;
            return "WATER";
        case POWER:
            // return topics[POWER].topic;
            return "POWER";
        case BATTERY:
            // return topics[BATTERY].topic;
            return "BATTERY";
        default:
            return ""; // Return an empty string for unknown data types
    }
}