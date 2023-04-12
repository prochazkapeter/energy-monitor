#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <PZEM004Tv30.h>

// Configuration
const char* ssid = "WB12-2G";
const char* password = "GymPB.2020";
const char* mqtt_server = "192.168.109.128";
#define mqtt_user "mqtt_user"
#define mqtt_password "csH2vABX8D"

// Public variables
PZEM004Tv30 pzem(Serial2, 16, 17);
int LED_BUIILTIN = 2;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUIILTIN, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
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
  if (String(topic) == "esp32/energymonitor") {
    if(messageTemp == "on"){
      digitalWrite(LED_BUIILTIN, HIGH);
    }
    else if(messageTemp == "off"){
      digitalWrite(LED_BUIILTIN, LOW);
    }
    else if(messageTemp == "reset"){
      if (pzem.resetEnergy())
        Serial.println("OK");
    }
    else if(messageTemp = "restartESP"){
      ESP.restart();
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("esp32_client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/energymonitor");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;

    // Read the data from the sensor
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();
    
    StaticJsonDocument<200> payload;
    payload["voltage"] = String(voltage, 1);
    payload["current"] = String(current, 3);
    payload["power"] = String(power, 3);
    payload["energy"] = String(energy, 2);
    payload["frequency"] = String(frequency, 1);
    payload["pfactor"] = String(pf, 2);

    String str_payload;
    serializeJson(payload, str_payload);

    Serial.println("Sending MQTT data");
    client.publish("homeenergy/sensor1", str_payload.c_str());
  }
}