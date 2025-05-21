#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define BATTERY_ADC_PIN 26
#define BATTERY_ADC_CHANNEL 0

void batteryInit(void);
int32_t batteryGetPercent();
int32_t batteryGetVoltageMv();

#ifdef __cplusplus
}
#endif