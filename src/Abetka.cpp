#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/uart.h"
#include "ws2812.h"
#include "lvgl.h"
#include "st7789.h"
#include "appMode.h"
#include "mifareHal.h"
#include "keyScan.h"
#include "battery.h"
#include <tusb.h>
#include "usb_mass_storage/tusb_config.h"
#include "sdAudio.h"
#include "sdCard.h"
#include "pinout.h"

// How to build without optimizations
// cmake -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1 ..
// ninja

#define BATTERY_CHECK_PERIOD_MS (60 * 1000)

// lcd configuration
static const struct st7789_config lcd_config = {
    .spi      = ST7789_SPI,
    .gpio_din = ST7789_DIN,
    .gpio_clk = ST7789_CLK,
    .gpio_cs  = ST7789_CS,
    .gpio_dc  = ST7789_DC,
    .gpio_rst = ST7789_RST,
    .gpio_bl  = ST7789_BL,
};
static const int lcd_width = 240;
static const int lcd_height = 240;
static uint16_t frameBuff[lcd_width * lcd_height] = {0};
static lv_display_t * display = NULL;
static uint32_t lastBatteryCheckMs = 0;
static int32_t batteryPercent = 100;

void display_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;
    size_t len = w * h * 2;

    st7789_caset(area->x1, area->x2);
    st7789_raset(area->y1, area->y2);
    st7789_ramwr();
    st7789_write(px_map, len);

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
    tud_init(BOARD_TUD_RHPORT); // should be called before the stdio_init_all()
    stdio_init_all();
    
    // TODO: O.T figure out how to utilize lvgl st7789 driver instead of custom one.
    st7789_init(&lcd_config, lcd_width, lcd_height);
    st7789_fill(0xFFFF);
    lv_init();
    display = lv_display_create(lcd_width, lcd_height);
    lv_display_set_buffers(display, (void *) frameBuff, NULL, sizeof(frameBuff), LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(display, display_flush_cb);
    lv_tick_set_cb(getTimeMs);
    
    ws2812Init();
    sdCardInit();
    mifareHalInit();
    sdAudioPlayFile((char *) "misc/powerOn.wav");
    
    // batteryInit();
    // batteryPercent = batteryGetPercent();
    if (batteryPercent <= 0) {
        appModeSwitch(APP_MODE_DISCHARGED);
    } else {
        appModeSwitch(APP_MODE_READ_CARD);
    }

    // TODO: O.T. Implement indication module (e.g. LED or screen feedback)
    // TODO: O.T. Add multiple example words for each letter
    // TODO: O.T fix ADC conflict with gpio pull-ups

    while(1)
    {
        keyScanProcess();
        // batteryProcess();
        appModeProcess();
        tud_task();
        lv_timer_handler();
        // TODO: O.T Add power detect, sleep cases USB enumeration to fail
        // sleep_ms(40);
    }
}
