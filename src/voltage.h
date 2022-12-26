#include <Arduino.h>

/* Returns RMS Voltage. The measurement itself takes time of one full period (1second / frequency). 
RMS method allow us to measure complex signals different from the perfect sine wave. */
float getVoltageAC();

/* This method reads the current value of the sensor and sets it as a reference point of measurement, and then returns this value.
No current must flow through the sensor at the time. */
int calibrate();
