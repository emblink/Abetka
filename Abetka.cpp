#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/uart.h"
#include "ws2812.h"
#include "dfplayer.h"
#include "lvgl.h"
#include "st7789.h"
#include "appMode.h"
#include "mifareHal.h"
#include "keyScan.h"
#include "battery.h"

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
#define ST7789_SPI      spi1

#define BATTERY_CHECK_PERIOD_MS (60 * 1000)

dfplayer_t dfplayer = {0};

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
static uint32_t lastBatteryCheckMs = 0;
static int32_t batteryPercent = 0;

static bool isBntPressed(uint gpio)
{
    bool isPressed = false == gpio_get(gpio);
    return isPressed;
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
static uint32_t getTimeMs(void) {
    return to_ms_since_boot(get_absolute_time());
}

void batteryProcess()
{
    if (getTimeMs() - lastBatteryCheckMs < BATTERY_CHECK_PERIOD_MS) {
        return;
    }

    batteryPercent = batteryGetPercent();
    if (batteryPercent <= 0) {
        appModeSwitch(APP_MODE_DISCHARGED);
    }
}

int main()
{
    stdio_init_all();

    // TODO: figure out how to utilize lvgl st7789 driver instead of custom one.
    st7789_init(&lcd_config, lcd_width, lcd_height);
    st7789_fill(0xFFFF);
    lv_init();
    display = lv_display_create(lcd_width, lcd_height);
    lv_display_set_buffers(display, (void *) frameBuff, NULL, sizeof(frameBuff), LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(display, display_flush_cb);
    lv_tick_set_cb(getTimeMs);

    mifareHalInit();

    ws2812Init();

    dfplayer_init(&dfplayer, DFPLAYER_UART, GPIO_TX, GPIO_RX);
    sleep_ms(10);
    dfplayer_set_volume(&dfplayer, 30);
    sleep_ms(10);
    dfplayer_set_eq(&dfplayer, EQ_BASS);
    sleep_ms(10);
    dfplayer_set_playback_mode(&dfplayer, MODE_FOLDER_REPEAT);
    sleep_ms(200);
    // dfplayer_play(&dfplayer, 2);

    batteryInit();
    batteryPercent = batteryGetPercent();
    if (batteryPercent <= 0) {
        appModeSwitch(APP_MODE_DISCHARGED);
    } else {
        appModeSwitch(APP_MODE_IDLE);
    }

    while(1)
    {
        keyScanProcess();
        batteryProcess();
        // TODO: add indication module
        // TODO: handle input in all states
        appModeProcess();
        lv_timer_handler();
        sleep_ms(40);
    }
}
