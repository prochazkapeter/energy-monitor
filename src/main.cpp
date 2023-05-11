#include "measurement.h"
#include "network.h"

// Public variables
PZEM004Tv30 pzem(Serial2, 16, 17);
WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wm;

long lastMsg = 0;
int cycles = 0;

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON, INPUT_PULLUP);
    if (digitalRead(BUTTON) == LOW) {
        wm.resetSettings();
    }
    digitalWrite(LED_PIN, HIGH);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    // Init and get the time
    configTime(3600, 3600, ntp_server);
    print_date("datefrom");
    print_date("billdate");
    digitalWrite(LED_PIN, LOW);
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED) {
        ESP.restart();
    }
    if (!client.connected()) {
        reconnect();
    }
    if (digitalRead(BUTTON) == LOW) {
        reset_wifi_button();
    }
    client.loop();

    long now = millis();
    // do measurement every 10 seconds
    if (now - lastMsg > 10000) {
        lastMsg = now;
        cycles++;

        // String str_payload = measure_data(); // TODO uncomment this line

        if (cycles > 6) { // every 1 minute
            cycles = 0;
            project_energy();
        }

        String str_payload = "Test string from ESP32 to Home Assistant"; // TODO remove
        Serial.println("Sending MQTT data");
        client.publish(mqtt_publish_topic, str_payload.c_str());
    }
}
