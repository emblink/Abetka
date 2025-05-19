#include "appModeRead.h"
#include "cardData.h"
#include "mfrc522.h"
#include "ws2812.h"
#include "dfplayer.h"
#include "mifareHal.h"
#include "keyScan.h"

static bool cardPresent = false;
extern dfplayer_t dfplayer;

static void processInput(Key key, KeyState event)
{
    if (KEY_LEFT == key) {
        if (KEY_STATE_PRESSED == event) {
            static bool toggle = false;
            if (toggle) {
                ws2812SetColor(RED);
            } else {
                ws2812SetColor(GREEN);
            }
            toggle = !toggle;
        } else if (KEY_STATE_HOLD == event) {
            ws2812SetColor(BLUE);
        } else if (KEY_STATE_LONG_HOLD == event) {
            ws2812SetColor(WHITE);
        }
    }
}

void appModeReadEnter()
{
    keyScanInit(processInput);
}

void appModeReadProcess()
{
    // TODO: fix card presence toggling
    bool isClose = mifareIsInProximity();

    if (isClose) {
        // It was already read one time, skip
        if (cardPresent) {
            return;
        }
    } else {
        // No card to read, skip
        cardPresent = false;
        return;
    }

    // TODO: read language and letter
    // TODO: play letter sound
    CardData cardData = {0};
    bool res = mifareReadData(cardData.rawData);
    if (!res)
    {
        printf("Failed to read Mifare data!\n");
        ws2812SetColor(RED);
        return;
    }
    
    cardPresent = true;
    if (LANGUAGE_ID_UA == cardData.langId) {
        printf("This is UA symbol: %u\n", cardData.symbol);
        if (cardData.symbol >= UA_ID_FIRST && cardData.symbol <= UA_ID_LAST) {
            dfplayer_play_folder(&dfplayer, cardData.langId, cardData.symbol - UA_ID_FIRST + 1); // offset 1 cause player track names starts with 001
            ws2812SetColor(GREEN);
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
}