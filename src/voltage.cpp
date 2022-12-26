#include "voltage.h"

#define VOLTAGE_ADC_GPIO 35 // pin number
#define FREQUENCY 50 // 50 Hz frequency
#define ADC_SCALE 4095.0
#define VREF 5.0
#define ZERO 2048

int zero = ZERO;
float sensitivity = 0.019;

float getVoltageAC() {
	uint32_t period = 1000000 / FREQUENCY;
	uint32_t t_start = micros();

	uint32_t Vsum = 0, measurements_count = 0;
	int32_t Vnow;

	while (micros() - t_start < period) {
		Vnow = analogRead(VOLTAGE_ADC_GPIO) - zero;
		Vsum += Vnow*Vnow;
		measurements_count++;
	}

	float Vrms = sqrt(Vsum / measurements_count) / ADC_SCALE * VREF / sensitivity;
	return Vrms;
}

int calibrate() {
	uint16_t acc = 0;
	for (int i = 0; i < 20; i++) {
		acc += analogRead(VOLTAGE_ADC_GPIO);
	}
	zero = acc / 20;
	return zero;
}
