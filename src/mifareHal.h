#pragma once
#include <stdbool.h>
#include "cardData.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MFRC522_SPI     spi0

void mifareHalInit();
bool mifareReadData(uint8_t *readBuff);
bool mifareWriteData(uint8_t *writeBuff);
bool mifareIsInProximity(void);

#ifdef __cplusplus
}
#endif
