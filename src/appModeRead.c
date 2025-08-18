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
#include "sdCard.h"

static CardData cardData = {};
static lv_obj_t *labelSymbol = NULL;
static Node *wordsList = NULL;
static Node *currentWord = NULL;
static lv_obj_t *imgWord = NULL;

static void playWordAnimation(const char *imgPath)
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

static void playWord(void)
{
    char folderPath[100] = {0};
    snprintf(folderPath, sizeof(folderPath), "%s/words/%s", cardData.langName, cardData.symbolUtf8);

    if (!currentWord) {
        return;
    }

    char path[100] = {0};
    snprintf(path, sizeof(path), "%s/%s.wav", folderPath, currentWord->name);
    sdAudioPlayFile(path);

    snprintf(path, sizeof(path), "A:/%s/%s.bmp", folderPath, currentWord->name);
    playWordAnimation(path);
}

static void updateUASymbolOnDisplay()
{
    lv_label_set_text(labelSymbol, cardData.symbolUtf8);
}

static void playLetter()
{
    char path[50] = {0};
    snprintf(path, sizeof(path), "%s/%s/%s.wav", cardData.langName, "letters", cardData.symbolUtf8);
    sdAudioPlayFile(path);

    // update words list
    if (wordsList) {
        freeList(wordsList);
        wordsList = NULL;
    }

    char folderPath[100] = {0};
    snprintf(folderPath, sizeof(folderPath), "%s/words/%s", cardData.langName, cardData.symbolUtf8);
    wordsList = sdCardGetFilesInFolder(folderPath, "wav");
    currentWord = wordsList;
}

static void playSound()
{
    char path[50] = {0};
    snprintf(path, sizeof(path), "%s/%s/%s.wav", cardData.langName, "sounds", cardData.symbolUtf8);
    sdAudioPlayFile(path);
}

static Node* getLastNode(Node* head)
{
    if (!head) return NULL;
    Node* node = head;
    while (node->next) {
        node = node->next;
    }
    return node;
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
    
    case KEY_LEFT:
        if (currentWord && KEY_STATE_PRESSED == event) {
            currentWord = currentWord->prev ? currentWord->prev : getLastNode(wordsList);
            playWord();
        }
        break;

    case KEY_RIGHT:
        if (currentWord && KEY_STATE_PRESSED == event) {
            currentWord = currentWord->next ? currentWord->next : wordsList;
            playWord();
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

    // Clear previous image before showing new symbol
    if (imgWord) {
        lv_obj_del(imgWord);
        imgWord = NULL;
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

    if (imgWord) {
        lv_obj_del(imgWord);
        imgWord = NULL;
    }
}