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
    // Configure GPIO for LED and user BUTTON
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON, INPUT_PULLUP);
    // If the user button is pressed immediately after boot,
    // reset Wi-Fi configuration
    if (digitalRead(BUTTON) == LOW) {
        wm.resetSettings();
    }

    // Connect to Wi-Fi and configure MQTT connection
    setup_wifi();

    // Init and get time from NTP
    configTime(3600, 3600, ntp_server);
    // Print billing period from flash memory
    print_date("datefrom");
    print_date("billdate");
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

        String str_payload = measure_data();

        if (cycles > 6) { // every 1 minute
            cycles = 0;
            project_energy();
        }

        // String str_payload = "Test string from ESP32 to Home Assistant";
        Serial.print("Sending data -> ");
        Serial.println(str_payload);
        client.publish(mqtt_publish_topic, str_payload.c_str());
    }
}
