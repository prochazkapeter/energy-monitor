#include "network.h"

extern WiFiClient espClient;
extern PubSubClient client;
extern WiFiManager wm;
extern Preferences flashSaver;

String broker_address;
String broker_port;
String client_id;
String client_username;
String client_pw;

// This function connects to a Wi-Fi
void setup_wifi()
{
    digitalWrite(LED_PIN, HIGH); // LED on indicates start of WiFi setup

    // Read MQTT parameters from memory
    broker_address = read_param("broker");
    broker_port = read_param("port");
    client_id = read_param("clientid");
    client_username = read_param("clientusername");
    client_pw = read_param("clientpw");

    // Custom parameters for MQTT
    WiFiManagerParameter mqtt_broker_field("mqtt_broker", "MQTT Broker Address", broker_address.c_str(), 20);
    wm.addParameter(&mqtt_broker_field);
    WiFiManagerParameter mqtt_port_field("mqtt_port", "MQTT Broker Port", broker_port.c_str(), 5);
    wm.addParameter(&mqtt_port_field);
    WiFiManagerParameter mqtt_clientid_field("mqtt_id", "Client ID", client_id.c_str(), 20);
    wm.addParameter(&mqtt_clientid_field);
    WiFiManagerParameter mqtt_username_field("mqtt_username", "Client Username", client_username.c_str(), 20);
    wm.addParameter(&mqtt_username_field);
    WiFiManagerParameter mqtt_password_field("mqtt_password", "Client Password", client_pw.c_str(), 20);
    wm.addParameter(&mqtt_password_field);

    // Try to connect to the last Wi-Fi, if failed, create config portal
    wm.setConnectTimeout(WIFI_CONNECTION_TIMEOUT);
    wm.setConfigPortalTimeout(WIFI_CONFIGPORTAL_TIMEOUT);
    bool res = wm.autoConnect(AP_NAME, AP_PASS);

    if (!res) {
        Serial.println("Failed to connect");
        ESP.restart();
    } else {
        // Store MQTT parameters to flash memory
        broker_address = mqtt_broker_field.getValue();
        broker_port = mqtt_port_field.getValue();
        client_id = mqtt_clientid_field.getValue();
        client_username = mqtt_username_field.getValue();
        client_pw = mqtt_password_field.getValue();
        save_param(broker_address, "broker");
        save_param(broker_port, "port");
        save_param(client_id, "clientid");
        save_param(client_username, "clientusername");
        save_param(client_pw, "clientpw");

        Serial.println("WiFi connected, IP address: ");
        Serial.println(WiFi.localIP());

        // Configure MQTT connection
        client.setServer(broker_address.c_str(), atoi(broker_port.c_str()));
        client.setCallback(callback);

        digitalWrite(LED_PIN, LOW); // LED off indicates end of WiFi setup
    }
}

// This function restores the MQTT connection
void reconnect()
{
    digitalWrite(LED_PIN, HIGH);
    int num_of_tries = 0;

    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(client_id.c_str(), client_username.c_str(), client_pw.c_str())) {
            Serial.println("connected");
            // Subscribe
            client.subscribe(mqtt_subscribe_topic);
        } else {
            num_of_tries++;
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
        if (num_of_tries >= 5) {
            ESP.restart();
        }
    }
    digitalWrite(LED_PIN, LOW);
}

// This function handles incoming messages over MQTT
void callback(char *topic, byte *message, unsigned int length)
{
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;

    for (int i = 0; i < length; i++) {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
    }
    Serial.println();

    // Changes the output state according to the message
    if (String(topic) == mqtt_subscribe_topic) {
        if (messageTemp == "on") {
            digitalWrite(LED_PIN, HIGH);
        } else if (messageTemp == "off") {
            digitalWrite(LED_PIN, LOW);
        } else if (messageTemp == "resetEnergy") {
            reset_energy();
        } else if (messageTemp == "resetWiFi") {
            wm.resetSettings();
            ESP.restart();
        } else if (messageTemp == "restartESP") {
            ESP.restart();
        } else if (strncmp(messageTemp.c_str(), "billdate-", 8) == 0) {
            save_date(messageTemp, "billdate");
        } else if (strncmp(messageTemp.c_str(), "datefrom-", 8) == 0) {
            save_date(messageTemp, "datefrom");
        }
    }
}

void reset_wifi_button()
{
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    wm.resetSettings();
    ESP.restart();
}

void save_param(String message, const char *location)
{
    flashSaver.begin("energymonitor", false); // init namespace for saving data
    Serial.print("Storing param to flash -> ");
    Serial.println(message);
    flashSaver.putString(location, message);
    flashSaver.end();
}

String read_param(const char *location)
{
    flashSaver.begin("energymonitor", false); // init namespace for saving data
    String param = flashSaver.getString(location, "");
    flashSaver.end();
    return param;
}
