#pragma once
#include <stdint.h>

#define CARD_LANGUAGE_PAGE  4
#define CARD_SYMBOL_PAGE  5

#pragma pack(push, 1)
typedef union {
    struct {
        char langName[4];       // e.g., "ukr", "eng" — null-terminated or padded
        char symbolUtf8[4];     // UTF-8 encoded symbol (up to 4 bytes)
    };
    uint8_t rawData[16];
} CardData;
#pragma pack(pop)