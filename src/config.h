#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <PZEM004Tv30.h>
#include "time.h"

// Configuration
const char* ssid = "FILL HERE";
const char* password = "FILL HERE";
const char* mqtt_server = "FILL HERE"; // running on Home Assistant server
const char* ntp_server = "tik.cesnet.cz";
#define mqtt_user "FILL HERE"
#define mqtt_password "FILL HERE"
#define mqtt_id "esp32_client_test" //"esp32_client"
#define mqtt_publish_topic "homeenergy/test" //"homeenergy/sensor1"