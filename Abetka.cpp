#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/uart.h"
#include "mfrc522.h"
#include "ws2812.h"
#include "dfplayer.h"

/**
 * @brief Pin definitions for the DFPlayer.
 * Another pair of TX/RX pins can be used, just
 * be sure to address the relative uart instance.
 */
#define GPIO_TX         8       // To RX on the player
#define GPIO_RX         9       // To TX on the player
#define DFPLAYER_UART   uart1
#define CARD_DATA_PAGE  4
#define USER_BTN        24

typedef enum {
    LANGUAGE_ID_UA = 0x01,
    LANGUAGE_ID_EN,
} LanguageId;

typedef enum {
    NUMBER_ID_0 = 0x00,
    NUMBER_ID_1 = 0x01,
    NUMBER_ID_2 = 0x02,
    NUMBER_ID_3 = 0x03,
    NUMBER_ID_4 = 0x04,
    NUMBER_ID_5 = 0x05,
    NUMBER_ID_6 = 0x06,
    NUMBER_ID_7 = 0x07,
    NUMBER_ID_8 = 0x08,
    NUMBER_ID_9 = 0x09,
} NumberId;

typedef struct {
    uint16_t langId;
    uint16_t symbol;
} CardData;

bool isUsrBntPressed()
{
    bool isPressed = false == gpio_get(USER_BTN);
    return isPressed;
}

/**
 * @brief Create an instance of the DFPlayer.
 */
dfplayer_t dfplayer;

int main()
{
    stdio_init_all();

    gpio_init(USER_BTN);
    gpio_set_dir(USER_BTN, GPIO_IN);
    gpio_pull_up(USER_BTN);

    while (isUsrBntPressed()); // detected as 0 at startup

    MFRC522Ptr_t mfrc = MFRC522_Init();
    PCD_Init(mfrc, spi0);

    ws2812Init();

    dfplayer_init(&dfplayer, DFPLAYER_UART, GPIO_TX, GPIO_RX);
    sleep_ms(10);
    dfplayer_set_volume(&dfplayer, 30);
    sleep_ms(10);
    dfplayer_set_eq(&dfplayer, EQ_BASS);
    sleep_ms(10);
    dfplayer_set_playback_mode(&dfplayer, MODE_FOLDER_REPEAT);
    sleep_ms(200);
    dfplayer_play(&dfplayer, 2);

    while(1)
    {
        //Wait for new card
        printf("Waiting for card\n\r");
        ws2812SetColor(BLACK);
        while(!PICC_IsNewCardPresent(mfrc));

        // //Select the card
        printf("Selecting card\n\r");
        PICC_ReadCardSerial(mfrc);

        if (isUsrBntPressed()) {
            // Write Page 4
            uint8_t writeBuff[16] = {0};
            CardData * cardData = (CardData *) writeBuff;
            cardData->langId = LANGUAGE_ID_UA;
            cardData->symbol = 9 + 1; // 0 - 9 numbers, + 1 is letter A
            StatusCode res = MIFARE_Write(mfrc, CARD_DATA_PAGE, writeBuff, 16);
            if (STATUS_OK == res) {
                dfplayer_play(&dfplayer, 2);
                printf("Page Write UA A\n");
                ws2812SetColor(GREEN);
            } else {
                printf("Failed to write Page.\n");
                ws2812SetColor(RED);
            }
            sleep_ms(2000);
        } else {
            // Read Page
            uint8_t size = 18;
            uint8_t readBuff[18] = {0};
            uint8_t page = CARD_DATA_PAGE;
            StatusCode res = MIFARE_Read(mfrc, page, readBuff, &size);
            if (STATUS_OK == res) {
                for (int i = 0; i < 16; i += 4) {
                    printf("Page %02u: %02X %02X %02X %02X\n", page, readBuff[i], readBuff[i + 1], 
                            readBuff[i + 2], readBuff[i+ 3]);
                    page++;
                }
            } else {
                printf("Failed to read Page.\n");
                ws2812SetColor(RED);
                sleep_ms(1000);
                continue;
            }

            CardData * cardData = (CardData *) readBuff;
            if (LANGUAGE_ID_UA == cardData->langId) {
                printf("This is UA symbol: %u\n", cardData->symbol);
                if (10 == cardData->symbol) {
                    dfplayer_play(&dfplayer, 1);
                    ws2812SetColor(GREEN);
                }
            } else {
                ws2812SetColor(RED);
            }

            sleep_ms(1000);
        }
    }
}
