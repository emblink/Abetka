#pragma once
#include <stdint.h>

#define CARD_DATA_PAGE  4

typedef enum {
    LANGUGAGE_ID_INVALID = 0x00,
    LANGUAGE_ID_UA,
    LANGUAGE_ID_EN,
} LanguageId;

typedef enum {
    UA_NUMBER_ID_0 = 0x00,
    UA_NUMBER_ID_9 = UA_NUMBER_ID_0 + 9,
    UA_ID_FIRST, // А
    UA_ID_LAST = UA_ID_FIRST + 33 - 1,
} UAId;

typedef enum {
    EN_NUMBER_ID_0 = 0x00,
    EN_NUMBER_ID_9 = EN_NUMBER_ID_0 + 9,
    EN_ID_FIRST, // А
    EN_ID_LAST = EN_ID_FIRST + 26 - 1,
} ENId;

typedef union {
    struct {
        uint16_t langId;
        uint16_t symbol;
    };
    uint8_t rawData[18];
} CardData;