#include "config.h"
#include "measurement.h"

void setup_wifi();
void reconnect();
void reset_wifi_button();
void callback(char *topic, byte *message, unsigned int length);
void save_param(String message, const char *location);
String read_param(const char *location);