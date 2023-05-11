#include "config.h"
#include "measurement.h"

void setup_wifi();
void reconnect();
void reset_wifi_button();
void callback(char *topic, byte *message, unsigned int length);