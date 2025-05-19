#include <stdio.h>
#include "appModeWrite.h"
#include "appModeRead.h"
#include "cardData.h"
#include "mifareHal.h"
#include "ws2812.h"
#include "dfplayer.h"
#include "lvgl.h"
#include "keyScan.h"

extern dfplayer_t dfplayer;

static LanguageId currentLanguage = LANGUAGE_ID_UA;
static uint16_t currentSymbolIdx = 0;

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
#define UA_LETTER_COUNT (sizeof(ukrAlphabet) / sizeof(ukrAlphabet[0]))

static lv_obj_t *labelSymbol = NULL;
static lv_obj_t *statusCircle = NULL;
static lv_timer_t *clearStatusTimer = NULL;

#define STATUS_TIMEOUT_MS 1000

// 🟦 UTF-8 конвертація з Unicode (до 3 байт)
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

// Чорний бекграунд екрана
static void set_black_background()
{
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
}

static void updateSymbolDisplay()
{
    if (!labelSymbol) {
        labelSymbol = lv_label_create(lv_scr_act());

        extern const lv_font_t lv_font_ukrainian_48;
        lv_obj_set_style_text_font(labelSymbol, &lv_font_ukrainian_48, 0);
        lv_obj_set_style_text_color(labelSymbol, lv_color_white(), 0);
        lv_obj_align(labelSymbol, LV_ALIGN_CENTER, 0, 0);
    }

    char symbolUtf8[5] = {0};
    if (currentSymbolIdx < UA_LETTER_COUNT) {
        unicode_to_utf8(ukrAlphabet[currentSymbolIdx], symbolUtf8);
        lv_label_set_text(labelSymbol, symbolUtf8);
    } else {
        lv_label_set_text(labelSymbol, "?");
    }
}

static void clearStatusCircle_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    if (statusCircle) {
        lv_obj_add_flag(statusCircle, LV_OBJ_FLAG_HIDDEN);
    }
}

static void showWriteStatus(bool success)
{
    if (!statusCircle) {
        statusCircle = lv_obj_create(lv_scr_act());
        lv_obj_set_size(statusCircle, 40, 40);
        lv_obj_set_style_radius(statusCircle, LV_RADIUS_CIRCLE, 0);
        lv_obj_align(statusCircle, LV_ALIGN_BOTTOM_MID, 0, -10);
    }

    lv_color_t color = success ? lv_palette_main(LV_PALETTE_GREEN) : lv_palette_main(LV_PALETTE_RED);
    lv_obj_set_style_bg_color(statusCircle, color, 0);
    lv_obj_clear_flag(statusCircle, LV_OBJ_FLAG_HIDDEN);

    if (clearStatusTimer) {
        lv_timer_del(clearStatusTimer);
    }

    clearStatusTimer = lv_timer_create(clearStatusCircle_cb, STATUS_TIMEOUT_MS, NULL);
}


static void processInput(Key key, KeyState state)
{

}

void appModeWriteEnter()
{
    keyScanInit(processInput);
    currentLanguage = LANGUAGE_ID_UA;
    currentSymbolIdx = 0;

    set_black_background();

    if (labelSymbol) {
        lv_obj_del(labelSymbol);
        labelSymbol = NULL;
    }

    if (statusCircle) {
        lv_obj_del(statusCircle);
        statusCircle = NULL;
    }

    updateSymbolDisplay();
}

void appModeWriteProcess()
{
    if (!mifareIsInProximity()) {
        return;
    }

    CardData cardData = {0};
    cardData.langId = currentLanguage;
    cardData.symbol = currentSymbolIdx + UA_ID_FIRST;

    bool res = mifareWriteData(cardData.rawData);
    if (res) {
        showWriteStatus(true);
        ws2812SetColor(GREEN);
        printf("Written: %u\n", cardData.symbol);

        currentSymbolIdx++;
        if (currentSymbolIdx >= UA_LETTER_COUNT) {
            currentSymbolIdx = 0;
        }

        updateSymbolDisplay();
    } else {
        showWriteStatus(false);
        ws2812SetColor(RED);
        printf("Write failed\n");
    }
}

void appModeWriteExit()
{
    keyScanDeinit();

    if (labelSymbol) {
        lv_obj_del(labelSymbol);
        labelSymbol = NULL;
    }

    if (statusCircle) {
        lv_obj_del(statusCircle);
        statusCircle = NULL;
    }

    if (clearStatusTimer) {
        lv_timer_del(clearStatusTimer);
        clearStatusTimer = NULL;
    }
}
