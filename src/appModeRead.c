#include "appModeRead.h"
#include "cardData.h"
#include "mfrc522.h"
#include "ws2812.h"
#include "dfplayer.h"
#include "mifareHal.h"
#include "keyScan.h"
#include "appMode.h"
#include "lvgl.h"
#include "uaSymbol.h"

extern dfplayer_t dfplayer;
static CardData cardData = {.langId = LANGUGAGE_ID_INVALID};
static lv_obj_t *labelSymbol = NULL;

static void updateUASymbolOnDisplay()
{
    if (LANGUGAGE_ID_INVALID == cardData.langId) {
        return;
    }

    char symbolUtf8[5] = {0};
    int idx = cardData.symbol - UA_ID_FIRST;
    if (idx < UA_LETTER_COUNT) {
        uaSymbolGetByIndex(idx, symbolUtf8);
        lv_label_set_text(labelSymbol, symbolUtf8);
    } else {
        lv_label_set_text(labelSymbol, "?");
    }
}

static void processInput(Key key, KeyState event)
{
    if (KEY_LEFT == key) {
        if (KEY_STATE_PRESSED == event) {
            if (cardData.langId != LANGUGAGE_ID_INVALID) {
                dfplayer_play_folder(&dfplayer, 3, cardData.symbol - UA_ID_FIRST + 1);
            }
        } else if (KEY_STATE_HOLD == event) {
            appModeSwitch(APP_MODE_WRITE_CARD);
        } else if (KEY_STATE_LONG_HOLD == event) {
            ws2812SetColor(WHITE);
        }
    }
}

static void guiInit(void)
{
    /*Change the active screen's background color*/
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Read mode");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    if (!labelSymbol) {
        labelSymbol = lv_label_create(lv_scr_act());
        extern const lv_font_t lv_font_ukrainian_48;
        lv_obj_set_style_text_font(labelSymbol, &lv_font_ukrainian_48, LV_PART_MAIN);
        lv_obj_set_style_text_color(labelSymbol, lv_color_white(), LV_PART_MAIN);
        lv_obj_align(labelSymbol, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(labelSymbol, "^_^");
    }
}

void appModeReadEnter()
{
    keyScanInit(processInput);
    ws2812SetColor(GREEN);
    guiInit();
}

void appModeReadProcess()
{
    bool isPresent = mifareIsInProximity();
    if (!isPresent) {
        return;
    }

    bool res = mifareReadData(cardData.rawData);
    if (!res)
    {
        printf("Failed to read Mifare data!\n");
        ws2812SetColor(RED);
        return;
    }

    if (LANGUAGE_ID_UA == cardData.langId) {
        printf("This is UA symbol: %u\n", cardData.symbol);
        if (cardData.symbol >= UA_ID_FIRST && cardData.symbol <= UA_ID_LAST) {
            dfplayer_play_folder(&dfplayer, cardData.langId, cardData.symbol - UA_ID_FIRST + 1); // offset 1 cause player track names starts with 001
            ws2812SetColor(GREEN);
            updateUASymbolOnDisplay();
        }
    } else if (LANGUAGE_ID_EN == cardData.langId) {
        printf("This is EN symbol: %u\n", cardData.symbol);
        if (cardData.symbol >= EN_ID_FIRST && cardData.symbol <= EN_ID_LAST) {
            dfplayer_play_folder(&dfplayer, cardData.langId, cardData.symbol - EN_ID_FIRST + 1); // offset 1 cause player track names starts with 001
            ws2812SetColor(GREEN);
        }
    } else {
        ws2812SetColor(RED);
    }
}

void appModeReadExit()
{
    keyScanDeinit();

    if (labelSymbol) {
        lv_obj_del(labelSymbol);
        labelSymbol = NULL;
    }
}