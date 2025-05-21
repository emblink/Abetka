#include "uaSymbol.h"
#include <stdint.h>

static const uint32_t ukrAlphabet[] = {
    0x410, // А
    0x411, // Б
    0x412, // В
    0x413, // Г
    0x490, // Ґ
    0x414, // Д
    0x415, // Е
    0x404, // Є
    0x416, // Ж
    0x417, // З
    0x418, // И
    0x406, // І
    0x407, // Ї
    0x419, // Й
    0x41A, // К
    0x41B, // Л
    0x41C, // М
    0x41D, // Н
    0x41E, // О
    0x41F, // П
    0x420, // Р
    0x421, // С
    0x422, // Т
    0x423, // У
    0x424, // Ф
    0x425, // Х
    0x426, // Ц
    0x427, // Ч
    0x428, // Ш
    0x429, // Щ
    0x42C, // Ь
    0x42E, // Ю
    0x42F  // Я
};

// UTF-8 to з Unicode (up to 3 bytes)
static void unicode_to_utf8(uint32_t codepoint, char *out)
{
    if (codepoint <= 0x7F) {
        out[0] = codepoint & 0x7F;
        out[1] = 0;
    } else if (codepoint <= 0x7FF) {
        out[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
        out[1] = 0x80 | (codepoint & 0x3F);
        out[2] = 0;
    } else {
        out[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
        out[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        out[2] = 0x80 | (codepoint & 0x3F);
        out[3] = 0;
    }
}

void uaSymbolGetByIndex(int idx, char symbolUtf8[5])
{
    if (idx >= 0 && idx < UA_LETTER_COUNT) {
        unicode_to_utf8(ukrAlphabet[idx], symbolUtf8);
    }
}
