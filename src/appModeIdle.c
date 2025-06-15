#include "appModeIdle.h"
#include "appMode.h"
#include "lvgl.h"
#include "mifareHal.h"
#include "keyScan.h"

static void processInput(Key key, KeyState event)
{
    if (KEY_STATE_PRESSED == event) {
        appModeSwitch(APP_MODE_READ_CARD);
    }
}

static void set_y_pos_cb(void * obj, int32_t v)
{
    lv_obj_set_y(obj, v);
}

static void set_opa_cb(void * obj, int32_t v)
{
    lv_obj_set_style_opa(obj, v, 0);
}

static void animation_ready_cb(lv_anim_t * anim)
{
    // Optional: Switch mode automatically after animation
    // appModeSwitch(APP_MODE_READ_CARD);
}

static lv_style_t gradient_style;

void appModeIdleEnter()
{
    keyScanInit(processInput);
    lv_disp_set_theme(NULL, lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_YELLOW), false, LV_FONT_DEFAULT));
    lv_obj_t *scr = lv_scr_act();

    // Setup gradient background
    lv_style_init(&gradient_style);
    lv_style_set_bg_color(&gradient_style, lv_color_make(0xFF, 0x00, 0x00)); // Red
    lv_style_set_bg_grad_color(&gradient_style, lv_color_make(0x00, 0x00, 0xFF)); // Blue
    lv_style_set_bg_grad_dir(&gradient_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&gradient_style, LV_OPA_COVER);
    lv_obj_add_style(lv_screen_active(), &gradient_style, 0);

    // Create and setup label for animation
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Абетка\nРомана");
    lv_obj_set_style_text_font(label, &lv_font_ukrainian_48, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN); 
    
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -240); // Initial position (off-screen top)
    lv_obj_set_style_opa(label, LV_OPA_TRANSP, 0); // Initial opacity

    // Setup Y position animation (slide in)
    lv_anim_t a_y;
    lv_anim_init(&a_y);
    lv_anim_set_var(&a_y, label);
    lv_anim_set_exec_cb(&a_y, set_y_pos_cb);
    lv_anim_set_time(&a_y, 1000); // 1 second duration
    lv_anim_set_delay(&a_y, 200); // 0.2 second delay
    lv_anim_set_values(&a_y, -240, 0); // From off-screen to center
    lv_anim_set_path_cb(&a_y, lv_anim_path_ease_out);

    // Setup opacity animation (fade in)
    lv_anim_t a_opa;
    lv_anim_init(&a_opa);
    lv_anim_set_var(&a_opa, label);
    lv_anim_set_exec_cb(&a_opa, set_opa_cb);
    lv_anim_set_time(&a_opa, 800); // 0.8 second duration
    lv_anim_set_delay(&a_opa, 0); // No delay
    lv_anim_set_values(&a_opa, LV_OPA_TRANSP, LV_OPA_COVER); // From transparent to opaque
    lv_anim_set_path_cb(&a_opa, lv_anim_path_linear);

    // Set animation completion callback
    lv_anim_set_ready_cb(&a_opa, animation_ready_cb); 

    // Start animations
    lv_anim_start(&a_y);
    lv_anim_start(&a_opa);
}

void appModeIdleProcess()
{
    // Animation handled by lv_timer_handler() in main loop
}

void appModeIdleExit()
{
    keyScanDeinit();
    lv_obj_remove_style(lv_screen_active(), &gradient_style, 0);
}
