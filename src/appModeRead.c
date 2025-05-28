#include "appModeRead.h"
#include "cardData.h"
#include "mfrc522.h"
#include "ws2812.h"
#include "mifareHal.h"
#include "keyScan.h"
#include "appMode.h"
#include "lvgl.h"
#include "uaSymbol.h"
#include "sdAudio.h"

static CardData cardData = {};
static lv_obj_t *labelSymbol = NULL;

static void updateUASymbolOnDisplay()
{
    lv_label_set_text(labelSymbol, cardData.symbolUtf8);
}

static void playLetter()
{
    // Form file path from language and symbol
    char path[50] = {0};
    snprintf(path, sizeof(path), "%s/%s/%s.wav", cardData.langName, "letters", cardData.symbolUtf8);
    sdAudioPlayFile(path);
}

static void playSound()
{
    // Form file path from language and symbol
    char path[50] = {0};
    snprintf(path, sizeof(path), "%s/%s/%s.wav", cardData.langName, "sounds", cardData.symbolUtf8);
    sdAudioPlayFile(path);
}

static void processInput(Key key, KeyState event)
{
    switch (key) {
    case KEY_SELECT:
        if (KEY_STATE_PRESSED == event) {
            playSound();
        } else if (KEY_STATE_HOLD == event) {
            appModeSwitch(APP_MODE_WRITE_CARD);
        }
        break;

        default:
            break;
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

    bool res = mifareReadData(&cardData);
    if (!res)
    {
        ws2812SetColor(RED);
        return;
    }

    updateUASymbolOnDisplay();
    playLetter();
    ws2812SetColor(GREEN);
}

void appModeReadExit()
{
    keyScanDeinit();

    if (labelSymbol) {
        lv_obj_del(labelSymbol);
        labelSymbol = NULL;
    }
}