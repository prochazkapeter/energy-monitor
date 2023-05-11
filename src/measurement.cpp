#include "measurement.h"

Preferences flashSaver;
extern PZEM004Tv30 pzem;
int projection = 0;

String measure_data(void)
{
    // Read the data from the sensor
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

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
    return str_payload;
}

// This function saves date in epoch format to flash memory
void save_date(String message, const char *location)
{
    if (message.length() == 17) {
        String date = message.substring(
            9, 17); // takes time in format <location-dd/mm/yy>
        struct tm date_from;
        date_from.tm_hour = 0;
        date_from.tm_min = 0;
        date_from.tm_sec = 0;
        date_from.tm_isdst = 1;
        date_from.tm_mday = atoi((date.substring(0, 2)).c_str());
        date_from.tm_mon = -1 + atoi((date.substring(3, 5)).c_str());
        date_from.tm_year = 100 + atoi((date.substring(6, 8)).c_str());
        uint32_t seconds = mktime(&date_from);
        flashSaver.begin("energymonitor",
                         false); // init namespace for saving data
        Serial.print("Storing time to flash -> ");
        Serial.println(seconds);
        flashSaver.putUInt(location, seconds);
        flashSaver.end();
    } else {
        Serial.println("ERROR: wrong date format...");
    }
}

// Checks flash memory for date record
void print_date(const char *location)
{
    flashSaver.begin("energymonitor", false); // init namespace for saving data
    time_t flash_datefrom = flashSaver.getUInt(location, 0);
    if (flash_datefrom != 0) {
        struct tm stored_date;
        localtime_r(&flash_datefrom, &stored_date);
        Serial.print("Readable date stored in flash: ");
        Serial.print(location);
        Serial.println(&stored_date, " %d/%m/%y");
    } else {
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
    if (flash_date != 0) {
        return flash_date;
    } else {
        Serial.println("INFO: bill date record not found");
        return 0;
    }
}

// Calculates estimated consumption
void project_energy(void)
{
    int energy = pzem.energy() * 1000;
    time_t date_from = read_date("datefrom");
    time_t bill_date = read_date("billdate");
    if (date_from != 0 && bill_date != 0) {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            Serial.println("ERROR: Failed to obtain time");
        } else {
            double now = mktime(&timeinfo);
            double remaining = bill_date - now;
            double elapsed = now - date_from;
            Serial.print("remaining hours: ");
            Serial.println(remaining / (3600));
            Serial.print("elapsed hours: ");
            Serial.println(elapsed / (3600));
            projection = energy * (1.0 + (remaining / elapsed));
            Serial.print("Projection in Wh: ");
            Serial.println(projection);
        }
    }
}

// Resets energy counter and writes new datefrom to flash memory
void reset_energy(void)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("ERROR: Failed to obtain time");
    } else {
        Serial.print("NTP time: ");
        Serial.println(&timeinfo, "%d/%m/%y");
        if (pzem.resetEnergy()) {
            uint32_t now = mktime(&timeinfo);
            flashSaver.begin("energymonitor",
                             false); // init namespace for saving data
            flashSaver.putUInt("datefrom", now);
            flashSaver.end();
            Serial.println("OK");
        } else {
            Serial.println("ERROR: reset energy failed!");
        }
    }
}
