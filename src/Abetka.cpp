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
#include "general.h"
#include <pico/bootrom.h>

// How to build without optimizations
// cmake -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=1 ..
// ninja

#define BATTERY_CHECK_PERIOD_MS (60 * 1000)
#define IDLE_TIMEOUT_MS (120 * 1000)

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
static uint32_t lastActionMs = 0;

static void idleTimerResetCallback(void)
{
    lastActionMs = getTimeMs();
}

static void idleTimeoutProcess(void)
{
    if (sdAudioIsPlaying()) {
        idleTimerResetCallback();
    }

    if (getTimeMs() - lastActionMs > IDLE_TIMEOUT_MS) {
        if (APP_MODE_SLEEP != appModeGetCurrent()) {
            appModeSwitch(APP_MODE_SLEEP);
        }
    }
}

static void updateBatteryPercent()
{
    static lv_obj_t * batLabel = NULL;
    if (NULL == batLabel) {
        batLabel = lv_label_create(lv_layer_top()); // special layer over any other screen
        lv_obj_set_style_text_font(batLabel, &lv_font_montserrat_14, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(batLabel, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_align(batLabel, LV_ALIGN_TOP_RIGHT, 0, 0);
    }

    if (batteryPercent >= 70) {
        lv_obj_set_style_bg_color(batLabel, lv_color_make(0, 0x80, 0), LV_PART_MAIN); // green
        lv_obj_set_style_text_color(batLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // white text
    } else if (batteryPercent >= 35) {
        lv_obj_set_style_bg_color(batLabel, lv_color_make(0xFF, 0xA5, 0x0), LV_PART_MAIN); // orange
        lv_obj_set_style_text_color(batLabel, lv_color_hex(0x0), LV_PART_MAIN); // black text
    } else {
        lv_obj_set_style_bg_color(batLabel, lv_color_make(0xFF, 0, 0), LV_PART_MAIN); // red
        lv_obj_set_style_text_color(batLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // white text
    }

    char batteryLableText[20] = {'\0'};
    sprintf(batteryLableText, "%d%%", batteryPercent);
    lv_label_set_text(batLabel, batteryLableText);
}

void batteryProcess()
{
    if (getTimeMs() - lastBatteryCheckMs < BATTERY_CHECK_PERIOD_MS) {
        return;
    }
    
    lastBatteryCheckMs = getTimeMs();
    batteryPercent = batteryGetPercent();
    if (batteryPercent <= 0) {
        appModeSwitch(APP_MODE_DISCHARGED);
    }

    updateBatteryPercent();
}

static void st7789SendCmdCb(lv_display_t * disp, const uint8_t * cmd, size_t cmd_size,
                            const uint8_t * param, size_t param_size)
{
    (void)disp;
    if (cmd_size != 1) return;

    st7789_cmd(cmd[0], param, param_size);
}

static void st7789SendColorCb(lv_display_t * disp, const uint8_t * cmd, size_t cmd_size,
                              uint8_t * param, size_t param_size)
{
    (void)cmd;
    (void)cmd_size;
    st7789_write(param, param_size);

    lv_display_flush_ready(disp);
}

static void checkBootloaderEnter(void)
{
    bool enterBootloader = keyScanGetKeyState(KEY_LEFT);
    uint32_t timeout = 0;
    #define BOOTLOADER_MODE_TIMEOUT_MS 1000
    while (enterBootloader && timeout < BOOTLOADER_MODE_TIMEOUT_MS) {
        enterBootloader = keyScanGetKeyState(KEY_LEFT);
        timeout++;
    }

    if (enterBootloader) {
        static lv_obj_t * label = NULL;
        lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
        label = lv_label_create(lv_screen_active());
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(label, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(label, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN); // green
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // white text
        lv_label_set_text(label, "USB Bootloader");
        lv_timer_handler();
        sleep_ms(250);
        reset_usb_boot(0, 0);
    }
}

int main()
{
    tud_init(BOARD_TUD_RHPORT); // should be called before the stdio_init_all()
    stdio_init_all();

    lv_init();
    lv_tick_set_cb(getTimeMs);
    st7789_init(&lcd_config, lcd_width, lcd_height);
    display = lv_st7789_create(
        lcd_width, lcd_height,
        LV_LCD_FLAG_NONE,
        st7789SendCmdCb,
        st7789SendColorCb
    );
    lv_disp_set_rotation(display, LV_DISPLAY_ROTATION_90);
    #define X_OFFSET 80
    lv_st7789_set_gap(display, X_OFFSET, 0);
    lv_st7789_set_invert(display, true);
    lv_st7789_set_gamma_curve(display, 2);
    lv_display_set_buffers(display, (void *) frameBuff, NULL, sizeof(frameBuff), LV_DISPLAY_RENDER_MODE_PARTIAL);

    ws2812Init();
    sdCardInit();
    mifareHalInit();
    keyScanSetIdleCallback(idleTimerResetCallback);
    
    batteryInit();
    batteryPercent = batteryGetPercent();
    updateBatteryPercent();
    if (batteryPercent <= 0) {
        appModeSwitch(APP_MODE_DISCHARGED);
    } else {
        checkBootloaderEnter();
        bool enterWriteMode = false;
        uint32_t timeout = 0;
        #define WRITE_MODE_TIMEOUT_MS 2000
        while (keyScanGetKeyState(KEY_RIGHT) && timeout < WRITE_MODE_TIMEOUT_MS) {
            timeout++;
            sleep_ms(1);
        }
        if (timeout >= WRITE_MODE_TIMEOUT_MS) {
            appModeSwitch(APP_MODE_WRITE_CARD);
        } else {
            appModeSwitch(APP_MODE_IDLE);
            sdAudioPlayFile((char *) "misc/powerOn.wav");
        }
    }
    
    // TODO: O.T add English assets

    while(1)
    {
        keyScanProcess();
        batteryProcess();
        appModeProcess();
        sdAudioProcess();
        idleTimeoutProcess();
        tud_task();
        lv_timer_handler();
        // TODO: O.T Add power detect, sleep cases USB enumeration to fail
        // sleep_ms(40);
    }
}
