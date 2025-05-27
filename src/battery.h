#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

void batteryInit(void);
int32_t batteryGetPercent();
int32_t batteryGetVoltageMv();

#ifdef __cplusplus
}
#endif