#include "appModeSleep.h"
#include "appMode.h"
#include "lvgl.h"
#include "keyScan.h"
#include "general.h"
#include "hardware/watchdog.h"
#include "pinout.h"
#include "hardware/timer.h"
#include "pico/sleep.h"

static bool enteringDeepSleep = false;
static uint32_t sleepStartMs = 0;

void appModeSleepEnter()
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "SLEEP");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); 

    sleepStartMs = getTimeMs();
}

void appModeSleepProcess()
{
    if (getTimeMs() - sleepStartMs > 500) {
        if (enteringDeepSleep) {
            sleep_run_from_dormant_source(DORMANT_SOURCE_ROSC);
            sleep_goto_dormant_until_pin(KEY_LEFT_GPIO, false, false);
            sleep_power_up();
            watchdog_reboot(0, 0, 0);
            for (;;) {}
        } else {
            sleepStartMs = getTimeMs();
            enteringDeepSleep = true;
        }
    }
}

void appModeSleepExit()
{

}
