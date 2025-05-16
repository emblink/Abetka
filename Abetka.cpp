#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/uart.h"
#include "hardware/timer.h"
#include "mfrc522.h"
#include "ws2812.h"
#include "dfplayer.h"
#include "lvgl.h"
#include "st7789.h"

// How to build without optimizations
// cmake -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1 ..
// ninja
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
#define MFRC522_SPI     spi0
#define ST7789_SPI      spi1

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

typedef union {
    struct {
        uint16_t langId;
        uint16_t symbol;
    };
    uint8_t rawData[18];
} CardData;

static MFRC522Ptr_t mfrc = NULL;
static dfplayer_t dfplayer = {0};

// lcd configuration
static const struct st7789_config lcd_config = {
    .spi      = ST7789_SPI,
    .gpio_din = 11,
    .gpio_clk = 10,
    .gpio_cs  = -1,
    .gpio_dc  = 13,
    .gpio_rst = 14,
    .gpio_bl  = 15,
};
static const int lcd_width = 240;
static const int lcd_height = 240;
static uint16_t frameBuff[lcd_width * lcd_height] = {0};
static lv_display_t * display = NULL;

static bool isUsrBntPressed()
{
    bool isPressed = false == gpio_get(USER_BTN);
    return isPressed;
}

void lv_example_hello(void)
{
    /*Change the active screen's background color*/
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Абетка\nРомана");
    lv_obj_set_style_text_font(label, &lv_font_ukrainian_48, LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
    // lv_obj_set_style_text_font(label, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void display_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    /* The most simple case (also the slowest) to send all rendered pixels to the
     * screen one-by-one.  `put_px` is just an example.  It needs to be implemented by you. */
    uint16_t * buf16 = (uint16_t *)px_map; /* Let's say it's a 16 bit (RGB565) display */
    int32_t x, y;
    st7789_set_cursor(0, 0);
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            st7789_put(*buf16);
            buf16++;
        }
    }
    // TODO: add fast write for the whole frame 

    /* IMPORTANT!!!
     * Inform LVGL that flushing is complete so buffer can be modified again. */
    lv_display_flush_ready(display);
}

// Millisecond timestamp — you need this for `lv_tick_inc`
uint32_t getTimeMs(void) {
    return to_ms_since_boot(get_absolute_time());
}

bool readMifareData(uint8_t * readBuff)
{
    printf("Read Mifare Data\n\r");
    PICC_ReadCardSerial(mfrc);
    // Read Page
    uint8_t size = 18;
    uint8_t page = CARD_DATA_PAGE;
    StatusCode res = MIFARE_Read(mfrc, page, readBuff, &size);
    if (STATUS_OK == res) {
        for (int i = 0; i < 16; i += 4) {
            printf("Page %02u: %02X %02X %02X %02X\n", page, readBuff[i], readBuff[i + 1], 
                    readBuff[i + 2], readBuff[i+ 3]);
            page++;
        }
    }

    return STATUS_OK == res;
}

bool isMifareInProximity()
{
    bool res = PICC_IsNewCardPresent(mfrc);
    return res;
}

void processMifareWriteMode()
{
    // TODO: Show language selection UA/EN
    // TODO: add input handling
    // If button is pressed show first letter
    // If card is in proximity rewrite it with the letter data
    // Write Page 4
    if (!isMifareInProximity())
    {
        return;
    }

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
        // TODO: play failed sound
        ws2812SetColor(RED);
    }
}

void processReadMode()
{
    if (!isMifareInProximity())
    {
        return;
    }
    
    // TODO: read language and letter
    // TODO: play letter sound
    CardData cardData = {0};
    bool res = readMifareData(cardData.rawData);
    if (!res)
    {
        printf("Failed to read Mifare data!\n");
        ws2812SetColor(RED);
        return;
    }

    if (LANGUAGE_ID_UA == cardData.langId) {
        printf("This is UA symbol: %u\n", cardData.symbol);
        if (10 == cardData.symbol) {
            dfplayer_play(&dfplayer, 1); // letter A
            ws2812SetColor(GREEN);
        }
    } else if (LANGUAGE_ID_EN == cardData.langId) {
        ws2812SetColor(RED);
    } else {
        ws2812SetColor(RED);
    }
}

#define BUTTON_GPIO 21
#define DEBOUNCE_SAMPLES 80
#define PRESSED_THRESHOLD (DEBOUNCE_SAMPLES / 2)
#define TIMER_INTERVAL_US 100
static volatile bool currentState = false; // true - pressed, false - released 
static volatile uint32_t debounceSamples = 0;
static volatile int32_t averageStateValue = 0;
static volatile bool processButtonChange = false;
static volatile bool debounceTimerActive = false;
static repeating_timer_t debounceTimer;

bool buttonTimerCallback(repeating_timer_t *t) {
    debounceSamples++;
    averageStateValue += (false == gpio_get(BUTTON_GPIO)) ? 1 : -1; // low is pressed

    if (debounceSamples >= DEBOUNCE_SAMPLES) {
        bool newState = false;
        if (averageStateValue >= PRESSED_THRESHOLD) {
            newState = true;
        }

        if (newState != currentState) {
            currentState = newState;
            processButtonChange = true;
        }

        debounceSamples = 0;
        averageStateValue = 0;
        cancel_repeating_timer(&debounceTimer);
        debounceTimerActive = false;
    }

    return debounceTimerActive;  // keep the timer active until explicitly canceled
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    if (!debounceTimerActive) {
        debounceTimerActive = true;
        debounceSamples = 0;
        averageStateValue = 0;
        add_repeating_timer_us(-TIMER_INTERVAL_US, buttonTimerCallback, NULL, &debounceTimer);
    }
}

void processInput()
{
    if (processButtonChange) {
        processButtonChange = false;
        if (currentState) {
            static bool toggle = false;
            if (toggle) {
                ws2812SetColor(GREEN);
            } else {
                ws2812SetColor(RED);
            }
            toggle = !toggle;
        }
        printf("Button %s\n", currentState ? "PRESSED" : "RELEASED");
    }
}

int main()
{
    stdio_init_all();

    gpio_init(USER_BTN);
    gpio_set_dir(USER_BTN, GPIO_IN);
    gpio_pull_up(USER_BTN);

    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);
    gpio_set_irq_enabled_with_callback(BUTTON_GPIO,  GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);

    while (isUsrBntPressed()); // detected as 0 at startup

    // TODO: figure out how to utilize lvgl st7789 driver instead of custom one.
    st7789_init(&lcd_config, lcd_width, lcd_height);
    st7789_fill(0xFFFF);
    lv_init();
    display = lv_display_create(lcd_width, lcd_height);
    lv_display_set_buffers(display, (void *) frameBuff, NULL, sizeof(frameBuff), LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(display, display_flush_cb);
    lv_tick_set_cb(getTimeMs);
    lv_example_hello();

    mfrc = MFRC522_Init();
    PCD_Init(mfrc, MFRC522_SPI);

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

    int pressedMs = 0;
    while (isUsrBntPressed() && pressedMs < 1000) {
        pressedMs++;
        sleep_ms(1);
        // Enter card write mode
        if (pressedMs >= 1000) {
            while (1)
            {
                processMifareWriteMode(); // no exit from this state
                sleep_ms(500);
            }
        }
    }

    while(1)
    {
        processInput();
        processReadMode();
        lv_timer_handler();
        sleep_ms(40);
    }
}
