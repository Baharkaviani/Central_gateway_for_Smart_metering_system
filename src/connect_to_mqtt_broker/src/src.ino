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
const char *ssid = "Warning";  // Enter your WiFi name
const char *password = "zilc8261"; // Enter WiFi password
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
unsigned long last_power_msg_time = 10000;
unsigned long last_battery_msg_time = 15000;

unsigned long elapsed_time_in_seconds = 0;

unsigned long updating_gas_data_period = 20000; // = 20 second
unsigned long updating_water_data_period = 20000; // = 20 second
unsigned long updating_power_data_period = 20000; // = 20 second
unsigned long updating_battery_data_period = 20000; // = 20 second

// Measured data to send
#define BUFFER_SIZE 10
char gas_consumption[BUFFER_SIZE][50];
char water_consumption[BUFFER_SIZE][50];
char power_consumption[BUFFER_SIZE][50];
char battery_consumption[BUFFER_SIZE][50];
int unsent_gas_index = 0;
int unsent_water_index = 0;
int unsent_power_index = 0;
int unsent_battery_index = 0;

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);

    // connecting to a WiFi network
    WiFiInit();

    // connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    reconnectBroker();

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
        reconnectBroker();
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

    // power
    if (elapsed_time_in_seconds - last_power_msg_time > updating_power_data_period) {
        last_power_msg_time = elapsed_time_in_seconds;
        send_power_consumption(elapsed_time_in_seconds);
    }

    // battery
    if (elapsed_time_in_seconds - last_battery_msg_time > updating_battery_data_period) {
        last_battery_msg_time = elapsed_time_in_seconds;
        send_battery_remaining(elapsed_time_in_seconds);
    }
}

void WiFiInit() {
    Serial.println("-----------------------WiFiInit-----------------------");
    // delete old config
    WiFi.disconnect(true);

    delay(1000);

    // handle different Wi-Fi events
    WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");
    Serial.println("------------------------------------------------------");
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
    Serial.println("-----------------------WiFiStationConnected-----------------------");
    Serial.println("Connected to AP successfully!");
    Serial.println("------------------------------------------------------------------");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
    Serial.println("-----------------------WiFiGotIP-----------------------");
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("-------------------------------------------------------");
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
    Serial.println("-----------------------WiFiStationDisconnected-----------------------");
    Serial.println("Disconnected from WiFi access point.");
    Serial.print("WiFi lost connection. Reason: ");
    Serial.println(info.wifi_sta_disconnected.reason);
    Serial.println("Trying to Reconnect...");
    WiFi.begin(ssid, password);
    Serial.println("---------------------------------------------------------------------");
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.println("-----------------------callback-----------------------");
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println("------------------------------------------------------");
}

void reconnectBroker() {
    Serial.println("-----------------------reconnectBroker-----------------------");
    // Loop until we're reconnected to mqtt broker
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
    Serial.println("-------------------------------------------------------------");
}

void send_gas_consumption(double gas_n) {
    Serial.println("-----------------------send_gas_consumption-----------------------");
    // convert the gas consumption value to a char array
    Serial.printf("unsent_gas_index = %d", unsent_gas_index);
    Serial.println();
    dtostrf(gas_n, 1, 2, gas_consumption[unsent_gas_index]);

    // print all gas_consumption datas
    for (int i = 0; i < BUFFER_SIZE; i++) {
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
    
    // create JSON object as a string
    String json_str = "{\"gas_consumption\": [";
    for (int i = 0; i <= unsent_gas_index; i++) {
        for (int j = 0; j < 50; j++) {
            if (gas_consumption[i][j] != '\0') {
                json_str += gas_consumption[i][j];
            }
            else {
                break;
            }
        }
        if (i != unsent_gas_index) {
            json_str += ",";
        }
    }
    json_str += "]}";

    Serial.println(json_str);
    Serial.print("time:");
    Serial.println(millis());

    // create char *payload
    char payload[json_str.length()];
    int i;
    for (i = 0; i < json_str.length(); i++) {
        payload[i] = json_str[i];
    }
    payload[i] = '\0';

    // publish
    bool is_sent = client.publish(gas_topic, payload);
    if (is_sent){
        unsent_gas_index = 0;
    }
    else {
        unsent_gas_index++;
    }
    Serial.println("------------------------------------------------------------------");
}

void send_water_consumption(double water_n) {
    Serial.println("-----------------------send_water_consumption-----------------------");
    // convert the water consumption value to a char array
    Serial.printf("unsent_water_index = %d", unsent_water_index);
    Serial.println();
    dtostrf(water_n, 1, 2, water_consumption[unsent_water_index]);

    // print all water_consumption datas
    for (int i = 0; i < BUFFER_SIZE; i++) {
        for (int j = 0; j < 50; j++) {
            if (water_consumption[i][j] == '\0') {
                Serial.println();
                break;
            }
            else{
                Serial.print(water_consumption[i][j]);
            }
        }
    }

    // create JSON object as a string
    String json_str = "{\"water_consumption\": [";
    for (int i = 0; i <= unsent_water_index; i++) {
        for (int j = 0; j < 50; j++) {
            if (water_consumption[i][j] != '\0') {
                json_str += water_consumption[i][j];
            }
            else {
                break;
            }
        }
        if (i != unsent_water_index) {
            json_str += ",";
        }
    }
    json_str += "]}";

    Serial.println(json_str);
    Serial.print("time:");
    Serial.println(millis());
    
    // create char *payload
    char payload[json_str.length()];
    int i;
    for (i = 0; i < json_str.length(); i++) {
        payload[i] = json_str[i];
    }
    payload[i] = '\0';

    // publish
    bool is_sent = client.publish(water_topic, payload);
    if (is_sent){
        unsent_water_index = 0;
    }
    else {
        unsent_water_index++;
    }
    Serial.println("--------------------------------------------------------------------");
}

void send_power_consumption(double power_n) {
    Serial.println("-----------------------send_power_consumption-----------------------");
    // convert the power consumption value to a char array
    Serial.printf("unsent_power_index = %d", unsent_power_index);
    Serial.println();
    dtostrf(power_n, 1, 2, power_consumption[unsent_power_index]);

    // print all power_consumption datas
    for (int i = 0; i < BUFFER_SIZE; i++) {
        for (int j = 0; j < 50; j++) {
            if (power_consumption[i][j] == '\0') {
                Serial.println();
                break;
            }
            else {
                Serial.print(power_consumption[i][j]);
            }
        }
    }

    // create JSON object as a string
    String json_str = "{\"power_consumption\": [";
    for (int i = 0; i <= unsent_power_index; i++) {
        for (int j = 0; j < 50; j++) {
            if (power_consumption[i][j] != '\0') {
                json_str += power_consumption[i][j];
            }
            else {
                break;
            }
        }
        if (i != unsent_power_index) {
            json_str += ",";
        }
    }
    json_str += "]}";
    
    Serial.println(json_str);
    Serial.print("time:");
    Serial.println(millis());

    // create char *payload
    char payload[json_str.length()];
    int i;
    for (int i = 0; i < json_str.length(); i++) {
        payload[i] = json_str[i];
    }
    payload[i] = '\0';

    // publish
    bool is_sent = client.publish(power_topic, payload);
    if (is_sent) {
        unsent_power_index = 0;
    }
    else {
        unsent_power_index++;
    }
    Serial.println("--------------------------------------------------------------------");
}

void send_battery_remaining(double battery_n) {
    Serial.println("-----------------------send_battery_remaining-----------------------");
    // convert the battery consumption value to a char array
    Serial.printf("unsent_battery_index = %d", unsent_battery_index);
    Serial.println();
    dtostrf(battery_n, 1, 2, battery_consumption[unsent_battery_index]);

    // print all battery_consumption datas
    for (int i = 0; i < BUFFER_SIZE; i++) {
        for (int j = 0; j < 50; j++) {
            if (battery_consumption[i][j] == '\0') {
                Serial.println();
                break;
            }
            else {
                Serial.print(battery_consumption[i][j]);
            }
        }
    }

    // create JSON object as a string
    String json_str = "{\"battery_consumption\": [";
    for (int i = 0; i <= unsent_battery_index; i++) {
        for (int j = 0; j < 50; j++) {
            if (battery_consumption[i][j] != '\0') {
                json_str += battery_consumption[i][j];
            }
            else {
                break;
            }
        }
        if (i != unsent_battery_index) {
            json_str += ",";
        }
    }
    json_str += "]}";

    Serial.println(json_str);
    Serial.print("time:");
    Serial.println(millis());

    // create char *payload
    char payload[json_str.length()];
    int i;
    for (i = 0; i < json_str.length(); i++) {
        payload[i] = json_str[i];
    }
    payload[i] = '\0';

    // publish
    bool is_sent = client.publish(battery_topic, payload);
    if (is_sent){
        unsent_battery_index = 0;
    }
    else {
        unsent_battery_index++;
    }
    Serial.println("--------------------------------------------------------------------");
}

void receive_command_text() {
    // TODO
}