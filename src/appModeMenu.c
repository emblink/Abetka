#include "appMode.h"
#include "lvgl.h"
#include "keyScan.h"
#include <stdio.h>
#include "sdAudio.h"
#include "sdCard.h"

typedef enum {
    MODE_MUSIC,
    MODE_CARD_READ,
    MODE_COUNT,
} Mode;

typedef struct {
    char assetName[50];
} ModeAssets;

static Mode volatile currentMode = MODE_COUNT;
static Mode nextMode = MODE_MUSIC;
static lv_obj_t *imgWord = NULL;
static const ModeAssets modeAssetsPath[MODE_COUNT] = {
    [MODE_MUSIC] = {"menu/music_mode"},
    [MODE_CARD_READ] = {"menu/card_mode"},
};

static void showMode(Mode mode);

static void processInput(Key key, KeyEvent event)
{
    if (KEY_EVENT_SHORT_PRESS != event) {
        return;
    }

    switch (key) {
    case KEY_LEFT:
        appModeSwitch(MODE_MUSIC == currentMode ? APP_MODE_MUSIC : APP_MODE_READ_CARD);
        break;
    case KEY_RIGHT:
        nextMode = (currentMode + 1) % MODE_COUNT;
        break;
    default:
        break;
    }
}

static void guiInit(void)
{
    /*Change the active screen's background color*/
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);
}

void appModeMenuEnter()
{
    nextMode = MODE_MUSIC;
    keyScanInit(processInput);
    guiInit();
    if (MODE_COUNT != currentMode) {
        showMode(currentMode);
    }
}

static void playAnimation(const char *imgPath)
{
    if (imgWord) {
        lv_obj_del(imgWord);
        imgWord = NULL;
    }

    imgWord = lv_img_create(lv_scr_act());
    lv_img_set_src(imgWord, imgPath);

    lv_obj_set_size(imgWord, 240, 240);
    lv_obj_align(imgWord, LV_ALIGN_CENTER, 0, 0);
}

static void showMode(Mode mode)
{
    char path[100] = {0};
    snprintf(path, sizeof(path), "A:/%s.bmp", modeAssetsPath[mode].assetName);
    playAnimation(path);

    snprintf(path, sizeof(path), "%s.wav", modeAssetsPath[mode].assetName);
    sdAudioPlayFile(path);
}

void appModeMenuProcess()
{
    if (currentMode == nextMode) {
        return;
    }

    switch (nextMode) {
    case MODE_MUSIC:
    case MODE_CARD_READ:
        showMode(nextMode);
        break;
    default:
        break;
    }

    currentMode = nextMode;
}

void appModeMenuExit()
{
    keyScanDeinit();
}
