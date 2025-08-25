#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "appModeWrite.h"
#include "appModeRead.h"
#include "cardData.h"
#include "mifareHal.h"
#include "ws2812.h"
#include "lvgl.h"
#include "keyScan.h"
#include "appMode.h"
#include "uaSymbol.h"
#include "sdCard.h"
#include "lv_conf.h"

typedef struct LabelNode {
    lv_obj_t *label_obj;
    struct LabelNode *next;
} LabelNode;

typedef enum {
    MENU_LANGUAGE,
    MENU_LETTERS,
    MENU_COUNT,
} Menu;

#define STATUS_TIMEOUT_MS 1000

static struct {
    lv_obj_t *selectedLetterLabel;
    lv_obj_t *statusCircle;
    lv_timer_t *clearStatusTimer;
    Node* letterListHead;
    Node* selectedLetter;
} menuLetter = {0};

static struct {
    Node *langListHead;
    Node *selectedLang;
    LabelNode *langLabelsHead; // Head of the linked list for LVGL label objects
} menuLanguage = {0};

static Menu currentMenu = MENU_COUNT;

static void clearStatusCircle_cb(lv_timer_t *t);
static void showWriteStatus(bool success);
static void updateLanguageListHighlight(void);
static void loadLetterList(const char* languageName);
static void processLanguageMenuInput(Key key, KeyState event);
static void processLettersMenuInput(Key key, KeyState event);
static void guiInit(void);
static void menuSwitch(Menu newMenu);
static void processLanguageMenu(void);
static void processLettersMenu(void);
static void menuLanguageEnter(void);
static void menuLanguageExit(void);
static void menuLetterEnter(void);
static void menuLetterExit(void);

void updateSelectedLetter(void)
{
    if (!menuLetter.selectedLetter || !menuLetter.selectedLetterLabel) {
        return;
    }

    if (menuLanguage.selectedLang && 0 == strcmp("ukr", menuLanguage.selectedLang->name)) {
        lv_obj_set_style_text_font(menuLetter.selectedLetterLabel, &lv_font_ukrainian_48, LV_PART_MAIN);
    } else {
        lv_obj_set_style_text_font(menuLetter.selectedLetterLabel, &lv_font_montserrat_48, LV_PART_MAIN);
    }
    lv_label_set_text(menuLetter.selectedLetterLabel, menuLetter.selectedLetter->name);
}

static void clearStatusCircle_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    if (menuLetter.statusCircle) {
        lv_obj_add_flag(menuLetter.statusCircle, LV_OBJ_FLAG_HIDDEN);
    }
}

static void showWriteStatus(bool success)
{
    lv_color_t color = success ? lv_palette_main(LV_PALETTE_GREEN) : lv_palette_main(LV_PALETTE_RED);
    lv_obj_set_style_bg_color(menuLetter.statusCircle, color, LV_PART_MAIN);
    lv_obj_clear_flag(menuLetter.statusCircle, LV_OBJ_FLAG_HIDDEN);

    if (menuLetter.clearStatusTimer) {
        lv_timer_reset(menuLetter.clearStatusTimer);
    } else {
        menuLetter.clearStatusTimer = lv_timer_create(clearStatusCircle_cb, STATUS_TIMEOUT_MS, NULL);
    }
}

static void updateLanguageListHighlight(void)
{
    // If no language folders are found, display a message
    if (!menuLanguage.langListHead) {
        lv_obj_t* message_label = lv_label_create(lv_scr_act());
        lv_label_set_text(message_label, "Not found!");
        lv_obj_set_style_text_color(message_label, lv_color_white(), LV_PART_MAIN);
        lv_obj_align(message_label, LV_ALIGN_CENTER, 0, 0);
        return;
    }

    Node *node = menuLanguage.langListHead;
    LabelNode *label_node = menuLanguage.langLabelsHead;
    while (node && label_node) {
        lv_label_set_text_fmt(label_node->label_obj, "%s%s",
                             (node == menuLanguage.selectedLang) ? "> " : "  ",
                             node->name);
        node = node->next;
        label_node = label_node->next;
    }
}

static void loadLetterList(const char* languageName)
{
    if (menuLetter.letterListHead) {
        menuLetter.letterListHead = NULL;
    }

    if (menuLetter.letterListHead) {
        freeList(menuLetter.letterListHead);
        menuLetter.letterListHead = NULL;
    }
    menuLetter.letterListHead = sdCardGetLetters(languageName);

    if (!menuLetter.letterListHead) {
        printf("No letters found for language: %s\n", languageName);
        menuLetter.selectedLetter = NULL;
        return;
    }

    menuLetter.selectedLetter = menuLetter.letterListHead;
    updateSelectedLetter();
}

static void showLanguageList(void)
{
    lv_obj_t* scr = lv_scr_act();
    Node *node = menuLanguage.langListHead;
    int index = 0;

    // Clear previous LVGL labels and free LabelNode memory
    LabelNode *current_label_node = menuLanguage.langLabelsHead;
    while (current_label_node) {
        LabelNode *temp = current_label_node;
        lv_obj_del(temp->label_obj);
        current_label_node = current_label_node->next;
        free(temp);
    }
    menuLanguage.langLabelsHead = NULL; // Reset head

    LabelNode *last_label_node = NULL;

    while (node) {
        LabelNode *new_label_node = (LabelNode *)malloc(sizeof(LabelNode));
        if (!new_label_node) {
            printf("Memory allocation failed for LabelNode\n");
            break;
        }
        new_label_node->next = NULL;

        lv_obj_t* label = lv_label_create(scr);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        
        lv_label_set_text(label, node->name); // Set text without highlight prefix here
        
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 20 + index * 30);
        
        new_label_node->label_obj = label;

        if (!menuLanguage.langLabelsHead) {
            menuLanguage.langLabelsHead = new_label_node;
        } else {
            last_label_node->next = new_label_node;
        }
        last_label_node = new_label_node;

        node = node->next;
        index++;
    }
    updateLanguageListHighlight();
}

static void guiInit(void)
{
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x003a57), LV_PART_MAIN);

    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Write mode");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
}

static void menuLettersProcess(void)
{
    if (!mifareIsInProximity()) {
        return;
    }

    if (!menuLanguage.selectedLang || !menuLetter.selectedLetter) {
        return;
    }

    CardData cardData = {0};
    strncpy(cardData.langName, menuLanguage.selectedLang->name, sizeof(cardData.langName) - 1);
    cardData.langName[sizeof(cardData.langName) - 1] = '\0';

    strncpy(cardData.symbolUtf8, menuLetter.selectedLetter->name, sizeof(cardData.symbolUtf8) - 1);
    cardData.symbolUtf8[sizeof(cardData.symbolUtf8) - 1] = '\0';

    bool res = mifareWriteData(&cardData);
    if (res) {
        showWriteStatus(true);
        ws2812SetColor(GREEN);

        if (menuLetter.selectedLetter->next) {
            menuLetter.selectedLetter = menuLetter.selectedLetter->next;
        } else {
            menuLetter.selectedLetter = menuLetter.letterListHead;
        }
        updateSelectedLetter();
    } else {
        showWriteStatus(false);
        ws2812SetColor(RED);
        printf("Write failed\n");
    }
}

static void processLanguageMenuInput(Key key, KeyState event)
{
    switch (key) {
    case KEY_LEFT:
        if (KEY_STATE_HOLD == event) {
            appModeSwitch(APP_MODE_MENU);
        } else if (KEY_STATE_PRESSED == event) {
            if (menuLanguage.selectedLang) {
                menuSwitch(MENU_LETTERS);
            }
        }
        break;
    case KEY_RIGHT:
        if (KEY_STATE_PRESSED == event) {
            if (menuLanguage.selectedLang && menuLanguage.selectedLang->next) {
                menuLanguage.selectedLang = menuLanguage.selectedLang->next;
            } else if (menuLanguage.langListHead) {
                menuLanguage.selectedLang = menuLanguage.langListHead;
            }
            updateLanguageListHighlight();
        }
        break;
    default:
        break;
    }
}

void menuLanguageEnter(void)
{
    guiInit();
    keyScanInit(processLanguageMenuInput);
    if (menuLanguage.langListHead) {
        freeList(menuLanguage.langListHead);
        menuLanguage.langListHead = NULL;
    }
    menuLanguage.langListHead = sdCardGetLanguageList();
    if (!menuLanguage.langListHead) {
        printf("No language folders found\n");
    } 
    menuLanguage.selectedLang = menuLanguage.langListHead;
    showLanguageList();
}

void menuLanguageExit(void)
{
    keyScanDeinit();
    LabelNode *current_label_node = menuLanguage.langLabelsHead;
    while (current_label_node) {
        LabelNode *temp = current_label_node;
        lv_obj_del(temp->label_obj);
        current_label_node = current_label_node->next;
        free(temp);
    }
    menuLanguage.langLabelsHead = NULL;

    if (menuLanguage.langListHead) {
        menuLanguage.langListHead = NULL;
    }
}

void processLettersMenuInput(Key key, KeyState event)
{
    switch (key) {
    case KEY_LEFT:
        if (KEY_STATE_HOLD == event) {
            menuSwitch(MENU_LANGUAGE);
        } else if (KEY_STATE_PRESSED == event) {
            if (menuLetter.selectedLetter && menuLetter.selectedLetter->prev) {
                menuLetter.selectedLetter = menuLetter.selectedLetter->prev;
            } else if (menuLetter.letterListHead) {
                Node * head = menuLetter.letterListHead;
                while (head && head->next) {
                    head = head->next;
                }
                menuLetter.selectedLetter = head;
            }
            updateSelectedLetter();
        }
        break;
    case KEY_RIGHT:
        if (KEY_STATE_PRESSED == event) {
            if (menuLetter.selectedLetter && menuLetter.selectedLetter->next) {
                menuLetter.selectedLetter = menuLetter.selectedLetter->next;
            } else if (menuLetter.letterListHead) {
                menuLetter.selectedLetter = menuLetter.letterListHead;
            }
            updateSelectedLetter();
        }
        break;
    default:
        break;
    }
}

void menuLetterEnter(void)
{
    guiInit();
    keyScanInit(processLettersMenuInput);

    if (!menuLetter.selectedLetterLabel) {
        menuLetter.selectedLetterLabel = lv_label_create(lv_scr_act());
        lv_obj_set_style_text_color(menuLetter.selectedLetterLabel, lv_color_white(), LV_PART_MAIN);
        lv_obj_align(menuLetter.selectedLetterLabel, LV_ALIGN_CENTER, 0, 0);
    }

    if (!menuLetter.statusCircle) {
        menuLetter.statusCircle = lv_obj_create(lv_scr_act());
        lv_obj_set_size(menuLetter.statusCircle, 40, 40);
        lv_obj_set_style_radius(menuLetter.statusCircle, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_align(menuLetter.statusCircle, LV_ALIGN_BOTTOM_MID, 0, -10);
        lv_obj_add_flag(menuLetter.statusCircle, LV_OBJ_FLAG_HIDDEN);
    }

    if (!menuLetter.clearStatusTimer) {
        menuLetter.clearStatusTimer = lv_timer_create(clearStatusCircle_cb, STATUS_TIMEOUT_MS, NULL);
    } else {
        lv_timer_set_period(menuLetter.clearStatusTimer, STATUS_TIMEOUT_MS);
        lv_timer_reset(menuLetter.clearStatusTimer);
    }

    loadLetterList(menuLanguage.selectedLang ? menuLanguage.selectedLang->name : "");
}

void menuLetterExit(void)
{
    keyScanDeinit();
    if (menuLetter.selectedLetterLabel) {
        lv_obj_del(menuLetter.selectedLetterLabel);
        menuLetter.selectedLetterLabel = NULL;
    }

    if (menuLetter.statusCircle) {
        lv_obj_del(menuLetter.statusCircle);
        menuLetter.statusCircle = NULL;
    }

    if (menuLetter.clearStatusTimer) {
        lv_timer_del(menuLetter.clearStatusTimer);
        menuLetter.clearStatusTimer = NULL;
    }

    if (menuLetter.letterListHead) {
        menuLetter.letterListHead = NULL;
    }
}

static void menuLanguageProcess()
{
    // Nothing to do here
}

static void menuSwitch(Menu newMenu)
{
    if (currentMenu == newMenu) {
        return;
    }

    switch (currentMenu) {
        case MENU_LANGUAGE:
            menuLanguageExit();
            break;
        case MENU_LETTERS:
            menuLetterExit();
            break;
        default:
            break;
    }
    
    currentMenu = newMenu;
    lv_obj_clean(lv_scr_act());
    
    switch (currentMenu) {
        case MENU_LANGUAGE:
            menuLanguageEnter();
            break;
        case MENU_LETTERS:
            menuLetterEnter();
            break;
        case MENU_COUNT:
            break;
        default:
            break;
    }
}

void appModeWriteEnter()
{
    guiInit();
    ws2812SetColor(BLUE);
    menuSwitch(MENU_LANGUAGE);
}

void appModeWriteProcess()
{
    if (currentMenu == MENU_LANGUAGE) {
        menuLanguageProcess();
    } else if (currentMenu == MENU_LETTERS) {
        menuLettersProcess();
    }
}

void appModeWriteExit()
{
    keyScanDeinit();
    menuSwitch(MENU_COUNT);
}