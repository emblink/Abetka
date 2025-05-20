#include <stdio.h>
#include "appModeWrite.h"
#include "appModeRead.h"
#include "cardData.h"
#include "mifareHal.h"
#include "ws2812.h"
#include "dfplayer.h"
#include "lvgl.h"
#include "keyScan.h"
#include "appMode.h"
#include "lvgl.h"
#include "uaSymbol.h"

extern dfplayer_t dfplayer;

static LanguageId currentLanguage = LANGUAGE_ID_UA;
static uint16_t currentSymbolIdx = 0;

static lv_obj_t *labelSymbol = NULL;
static lv_obj_t *statusCircle = NULL;
static lv_timer_t *clearStatusTimer = NULL;

#define STATUS_TIMEOUT_MS 1000

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
        uaSymbolGetByIndex(currentSymbolIdx, symbolUtf8);
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

static void processInput(Key key, KeyState event)
{
    if (KEY_LEFT == key && KEY_STATE_PRESSED == event) {
        currentSymbolIdx++;
        if (currentSymbolIdx >= UA_LETTER_COUNT) {
            currentSymbolIdx = 0;
        }
        updateSymbolDisplay();
    } else if (KEY_LEFT == key && KEY_STATE_HOLD == event) {
        appModeSwitch(APP_MODE_READ_CARD);
    }
}

static void guiInit(void)
{
    /*Change the active screen's background color*/
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Write mode");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
}

void appModeWriteEnter()
{
    keyScanInit(processInput);
    currentLanguage = LANGUAGE_ID_UA;
    currentSymbolIdx = 0;

    set_black_background();
    ws2812SetColor(BLUE);

    if (labelSymbol) {
        lv_obj_del(labelSymbol);
        labelSymbol = NULL;
    }

    if (statusCircle) {
        lv_obj_del(statusCircle);
        statusCircle = NULL;
    }
    guiInit();

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
