#pragma once
#include <stdint.h>

#define CARD_DATA_PAGE  4

typedef union {
    struct {
        char langName[8]; // "ukr", "eng" etc. (null-terminated) https://en.wikipedia.org/wiki/List_of_ISO_639_language_codes
        char symbolUtf8[4]; // UTF-8 symbol (letter, number, symbol) — up to 4 bytes
        uint8_t reserved[6];
    };
    uint8_t rawData[18];
} CardData;