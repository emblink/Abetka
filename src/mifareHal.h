#pragma once
#include <stdbool.h>
#include "cardData.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define MFRC522_SPI     spi0

void mifareHalInit();
bool mifareReadData(CardData *card);
bool mifareWriteData(CardData *card);
bool mifareIsInProximity(void);

#ifdef __cplusplus
}
#endif
