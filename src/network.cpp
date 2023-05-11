#include "network.h"

extern WiFiClient espClient;
extern PubSubClient client;
extern WiFiManager wm;

// This function connects to a Wi-Fi
void setup_wifi()
{
    // We start by connecting to a WiFi network
    wm.setConnectTimeout(WIFI_CONNECTION_TIMEOUT);
    wm.setConfigPortalTimeout(WIFI_CONFIGPORTAL_TIMEOUT);
    bool res;
    res = wm.autoConnect(AP_NAME, AP_PASS);

    if (!res) {
        Serial.println("Failed to connect");
        ESP.restart();
    } else {
        // if you get here you have connected to the WiFi
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        digitalWrite(LED_PIN, LOW);
    }
}

// This function restores the MQTT connection
void reconnect()
{
    int num_of_tries = 0;

    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(mqtt_id, mqtt_user, mqtt_password)) {
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
