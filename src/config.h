#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define DEVICE "ESP32"
// WiFi AP SSID
#define WIFI_SSID "FILL HERE"
// WiFi password
#define WIFI_PASSWORD "FILL HERE"

#define INFLUXDB_URL "FILL HERE"
#define INFLUXDB_TOKEN "FILL HERE"
#define INFLUXDB_ORG "FILL HERE"
#define INFLUXDB_BUCKET "FILL HERE"

// Time zone info
#define TZ_INFO "UTC1"

// NTP servers
#define NTP1 "ntp.nic.cz"
#define NTP2 "tik.cesnet.cz"