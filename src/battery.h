#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define BATTERY_ADC_PIN 29
#define BATTERY_ADC_CHANNEL 3

void batteryInit(void);
int32_t batteryGetPercent();
int32_t batteryGetVoltageMv();

#ifdef __cplusplus
}
#endif