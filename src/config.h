#include "time.h"
#include <ArduinoJson.h>
#include <PZEM004Tv30.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <Wire.h>

#define LED_PIN 2
#define BUTTON 18

// Configuration
#define ntp_server "ntp.nic.cz"
#define AP_NAME "Energy Monitor"
#define AP_PASS "emonitor"
#define mqtt_publish_topic "homeenergy/sensor1" // "homeenergy/test"
#define mqtt_subscribe_topic "esp32/sensor1"    // "esp32/test"
#define WIFI_CONNECTION_TIMEOUT 120
#define WIFI_CONFIGPORTAL_TIMEOUT 60