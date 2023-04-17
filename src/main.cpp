#include "config.h"

// Public variables
PZEM004Tv30 pzem(Serial2, 16, 17);
WiFiClient espClient;
PubSubClient client(espClient);
Preferences flashSaver;

int LED_BUIILTIN = 2;
long lastMsg = 0;
int cycles = 0;
int projection = 0;

void setup_wifi();
void callback(char *topic, byte *message, unsigned int length);
void reconnect();
void save_billdate(String message, const char *location);
void print_date(const char *location);
time_t read_date(const char *date);
int project_energy(int energy);
void reset_energy(void);

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUIILTIN, OUTPUT);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    // Init and get the time
    configTime(3600, 3600, ntp_server);
    print_date("datefrom");
    print_date("billdate");
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connection to WiFi lost. Reconnecting...");
        WiFi.disconnect();
        setup_wifi();
    }
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 10000)
    {
        lastMsg = now;
        cycles++;

        // Read the data from the sensor
        float voltage = pzem.voltage();
        float current = pzem.current();
        float power = pzem.power();
        float energy = pzem.energy();
        float frequency = pzem.frequency();
        float pf = pzem.pf();

        if (cycles > 6)
        { // every 1 minute
            projection = project_energy(energy * 1000);
            Serial.print("Projection in Wh: ");
            Serial.println(projection);
        }

        StaticJsonDocument<800> payload;
        payload["voltage"] = String(voltage, 1);
        payload["current"] = String(current, 3);
        payload["power"] = String(power, 3);
        payload["energy"] = String(energy, 2);
        payload["projection"] = String(projection);
        payload["frequency"] = String(frequency, 1);
        payload["pfactor"] = String(pf, 2);

        String str_payload;
        serializeJson(payload, str_payload);

        Serial.println("Sending MQTT data");
        client.publish(mqtt_publish_topic, str_payload.c_str());
    }
}

// ------------------------- FUNCTIONS DEFINITON -------------------------------------------------

// This function connects to a Wi-Fi
void setup_wifi()
{
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

// This function restores the MQTT connection
void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(mqtt_id, mqtt_user, mqtt_password))
        {
            Serial.println("connected");
            // Subscribe
            client.subscribe(mqtt_subscribe_topic);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
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

    for (int i = 0; i < length; i++)
    {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
    }
    Serial.println();

    // Changes the output state according to the message
    if (String(topic) == mqtt_subscribe_topic)
    {
        if (messageTemp == "on")
        {
            digitalWrite(LED_BUIILTIN, HIGH);
        }
        else if (messageTemp == "off")
        {
            digitalWrite(LED_BUIILTIN, LOW);
        }
        else if (messageTemp == "reset")
        {
            reset_energy();
        }
        else if (messageTemp == "restartESP")
        {
            ESP.restart();
        }
        else if (strncmp(messageTemp.c_str(), "billdate-", 8) == 0)
        {
            save_billdate(messageTemp, "billdate");
        }
        else if (strncmp(messageTemp.c_str(), "datefrom-", 8) == 0)
        {
            save_billdate(messageTemp, "datefrom");
        }
    }
}

// This function saves date in epoch format to flash memory
void save_billdate(String message, const char *location)
{
    if (message.length() == 17)
    {
        String date = message.substring(9, 17); // takes time in format <location-dd/mm/yy>
        struct tm date_from;
        date_from.tm_hour = 0;
        date_from.tm_min = 0;
        date_from.tm_sec = 0;
        date_from.tm_isdst = 1;
        date_from.tm_mday = atoi((date.substring(0, 2)).c_str());
        date_from.tm_mon = -1 + atoi((date.substring(3, 5)).c_str());
        date_from.tm_year = 100 + atoi((date.substring(6, 8)).c_str());
        uint32_t seconds = mktime(&date_from);
        flashSaver.begin("energymonitor", false); // init namespace for saving data
        Serial.print("Storing time to flash -> ");
        Serial.println(seconds);
        flashSaver.putUInt(location, seconds);
        flashSaver.end();
    }
    else
    {
        Serial.println("ERROR: wrong date format...");
    }
}

// Checks flash memory for date record
void print_date(const char *location)
{
    flashSaver.begin("energymonitor", false); // init namespace for saving data
    time_t flash_datefrom = flashSaver.getUInt(location, 0);
    if (flash_datefrom != 0)
    {
        struct tm stored_date;
        localtime_r(&flash_datefrom, &stored_date);
        Serial.print("Readable date stored in flash: ");
        Serial.print(location);
        Serial.println(&stored_date, " %d/%m/%y");
    }
    else
    {
        Serial.println("INFO: bill date record not found");
    }
    flashSaver.end();
}

// Returns date from flash memory
time_t read_date(const char *date)
{
    flashSaver.begin("energymonitor", false); // init namespace for saving data
    time_t flash_date = flashSaver.getUInt(date, 0);
    flashSaver.end();
    if (flash_date != 0)
    {
        return flash_date;
    }
    else
    {
        Serial.println("INFO: bill date record not found");
        return 0;
    }
}

// Calculates estimated consumption
int project_energy(int energy)
{
    cycles = 0;
    int retval = 0;
    time_t date_from = read_date("datefrom");
    time_t bill_date = read_date("billdate");
    if (date_from != 0 && bill_date != 0)
    {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("ERROR: Failed to obtain time");
        }
        else
        {
            double now = mktime(&timeinfo);
            double remaining = bill_date - now;
            double elapsed = now - date_from;
            Serial.print("remaining hours: ");
            Serial.println(remaining / (3600));
            Serial.print("elapsed hours: ");
            Serial.println(elapsed / (3600));
            retval = energy * (1.0 + (remaining / elapsed));
        }
    }
    return retval;
}

// Resets energy counter and writes new datefrom to flash memory
void reset_energy(void)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("ERROR: Failed to obtain time");
    }
    else
    {
        Serial.print("NTP time: ");
        Serial.println(&timeinfo, "%d/%m/%y");
        if (pzem.resetEnergy())
        {
            uint32_t now = mktime(&timeinfo);
            flashSaver.begin("energymonitor", false); // init namespace for saving data
            flashSaver.putUInt("datefrom", now);
            flashSaver.end();
            Serial.println("OK");
        }
        else
        {
            Serial.println("ERROR: reset energy failed!");
        }
    }
}
