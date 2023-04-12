#include "config.h"

// Public variables
PZEM004Tv30 pzem(Serial2, 16, 17);
WiFiClient espClient;
PubSubClient client(espClient);
Preferences flashSaver;

int LED_BUIILTIN = 2;
long lastMsg = 0;

void setup_wifi();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();

void setup() {
  Serial.begin(115200);
  flashSaver.begin("energymonitor", false); // init namespace for saving data
  pinMode(LED_BUIILTIN, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Init and get the time
  configTime(3600, 3600, ntp_server);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("Connection to WiFi lost. Reconnecting...");
    WiFi.disconnect();
    setup_wifi();
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;

    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
    } else{
      struct tm start_date;
      double seconds;
      start_date.tm_hour = 0; start_date.tm_min = 0; start_date.tm_sec = 0;
      start_date.tm_mon = 0;  start_date.tm_mday = 1; start_date.tm_year = 123;
      Serial.println(&timeinfo, "%m/%d/%y %H:%M:%S");
      Serial.println(mktime(&timeinfo));
      seconds = difftime(mktime(&timeinfo), mktime(&start_date));
      double days = seconds / (3600 * 24);
      Serial.print("days difference: "); Serial.println(days);
    }

    // Read the data from the sensor
    /*
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
    */
    String str_payload;
    str_payload = "Test string over MQTT.";

    Serial.println("Sending MQTT data");
    client.publish(mqtt_publish_topic, str_payload.c_str());
  }
}

// ------------------------- FUNCTIONS DEFINITON -------------------------------------------------

// This function connects to a Wi-Fi
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

// This function restores the MQTT connection
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_id, mqtt_user, mqtt_password)) {
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

// This function handles incoming messages over MQTT
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