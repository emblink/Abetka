#include "appModeDischarged.h"
#include "appMode.h"
#include "lvgl.h"
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include "general.h"

// Static variables to manage the state
static lv_obj_t *discharged_label = NULL;
static lv_style_t discharged_style;
static uint64_t process_start_time_us = 0; // Use microseconds for high-resolution timing

/**
 * @brief Enter the discharged mode.
 * * This function sets up the screen to display a "low battery" warning.
 */
void appModeDischargedEnter()
{
    // Initialize the style for the warning background
    lv_style_init(&discharged_style);
    lv_style_set_bg_color(&discharged_style, lv_color_hex(0xCC0000)); // Red background for warning
    lv_style_set_bg_grad_color(&discharged_style, lv_color_hex(0x660000)); // Darker red for gradient
    lv_style_set_bg_grad_dir(&discharged_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&discharged_style, LV_OPA_COVER);
    lv_obj_add_style(lv_screen_active(), &discharged_style, 0);

    // Create a label to show the warning message
    discharged_label = lv_label_create(lv_screen_active());
    lv_label_set_text(discharged_label, "Low\nBattery");
    
    // Set a larger font and white text color for visibility
    lv_obj_set_style_text_font(discharged_label, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_set_style_text_color(discharged_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 

    lv_obj_align(discharged_label, LV_ALIGN_CENTER, 0, 0);
    // Set the text alignment within the label to center
    lv_obj_set_style_text_align(discharged_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    // Record the start time for the timeout
    process_start_time_us = getTimeMs();
}

/**
 * @brief Process logic for the discharged mode.
 * * This function checks for a 5-second timeout and switches to sleep mode after it expires.
 */
void appModeDischargedProcess()
{
    // Check if timeout have passed
    if ((getTimeMs() - process_start_time_us) > 2000) {
        appModeSwitch(APP_MODE_SLEEP); // Switch to a sleep mode
    }
}

/**
 * @brief Exit the discharged mode.
 * * This function cleans up the UI and transitions the device to a low-power state.
 */
void appModeDischargedExit()
{
    // Clean up LVGL objects and styles to free memory
    if (discharged_label != NULL) {
        lv_obj_del(discharged_label);
        discharged_label = NULL;
    }
    lv_obj_remove_style(lv_screen_active(), &discharged_style, 0);
    lv_style_reset(&discharged_style);
}