#include "appMode.h"
#include "lvgl.h"
#include "keyScan.h"
#include <stdio.h>
#include "sdAudio.h"
#include "sdCard.h"

static lv_obj_t *songLable = NULL;
static Node *songList = NULL;
static Node *currentSong = NULL;
static lv_obj_t *imgWord = NULL;
static volatile bool switchSongFlag = false;
static volatile bool playSongFlag = false;
const char *folderPath = "songs";

static void showSongCoverArt(const char *songName);

static void processInput(Key key, KeyState event)
{
    switch (key) {
    case KEY_LEFT:
        if (KEY_STATE_HOLD == event) {
            appModeSwitch(APP_MODE_MENU);
        } else if (KEY_STATE_PRESSED == event) {
            playSongFlag = true;
        }
        break;
    case KEY_RIGHT:
        if (KEY_STATE_PRESSED == event) {
            switchSongFlag = true;
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
}

void appModeMusicEnter()
{
    keyScanInit(processInput);
    guiInit();

    // update song list
    if (songList) {
        freeList(songList);
        songList = NULL;
    }

    songList = sdCardGetFilesInFolder(folderPath, "wav");
    currentSong = songList;
    showSongCoverArt(currentSong->name);
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

static void playSong(const char *songName)
{
    char path[100] = {0};
    snprintf(path, sizeof(path), "%s/%s.wav", folderPath, songName);
    sdAudioPlayFile(path);
}

static void showSongCoverArt(const char *songName)
{
    char path[100] = {0};
    snprintf(path, sizeof(path), "A:/%s/%s.bmp", folderPath, songName);
    playAnimation(path);

    if (songLable) {
        lv_obj_del(songLable);
        songLable = NULL;
    }

    songLable = lv_label_create(lv_scr_act());
    extern const lv_font_t lv_font_montserrat_14;
    lv_obj_set_style_text_font(songLable, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(songLable, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(songLable, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_bg_color(songLable, lv_color_make(0x80, 0x80, 0x80), LV_PART_MAIN); // grey

    lv_obj_align(songLable, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_label_set_text(songLable, songName);
}

void appModeMusicProcess()
{
    if (switchSongFlag) {
        switchSongFlag = false;
        if (sdAudioIsPlaying()) {
            sdAudioStop();
        }
        currentSong = currentSong->next ? currentSong->next : songList;
        showSongCoverArt(currentSong->name);
    }

    if (playSongFlag) {
        playSongFlag = false;
        if (sdAudioIsPlaying()) {
            sdAudioStop();
        } else {
            playSong(currentSong->name);
        }
    }
}

void appModeMusicExit()
{
    keyScanDeinit();

    if (songLable) {
        lv_obj_del(songLable);
        songLable = NULL;
    }

    if (imgWord) {
        lv_obj_del(imgWord);
        imgWord = NULL;
    }

    if (sdAudioIsPlaying()) {
        sdAudioStop();
    }
}
