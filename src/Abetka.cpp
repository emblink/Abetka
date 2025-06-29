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
    lv_display_set_buffers(display, (void *) frameBuff, NULL, sizeof(frameBuff), LV_DISPLAY_RENDER_MODE_FULL);

    ws2812Init();
    sdCardInit();
    mifareHalInit();
    sdAudioPlayFile((char *) "misc/powerOn.wav");
    
    batteryInit();
    batteryPercent = batteryGetPercent();
    if (batteryPercent <= 0) {
        appModeSwitch(APP_MODE_DISCHARGED);
    } else {
        appModeSwitch(APP_MODE_IDLE);
    }

    // TODO: O.T. Implement indication module (e.g. LED or screen feedback)
    // TODO: O.T. Add multiple example words for each letter
    // TODO: O.T add idle timeout and sleep mode
    // TODO: O.T fix blocking audio, consider to use another i2s module, links
    // TODO: O.T fix display flickering during audio playback
    // https://github.com/raspberrypi/pico-extras/tree/master/src/rp2_common/pico_audio_i2s
    // https://github.com/raspberrypi/pico-playground/blob/master/audio/sine_wave/sine_wave.c

    while(1)
    {
        keyScanProcess();
        batteryProcess();
        appModeProcess();
        sdAudioProcess();
        tud_task();
        lv_timer_handler();
        // TODO: O.T Add power detect, sleep cases USB enumeration to fail
        // sleep_ms(40);
    }
}
