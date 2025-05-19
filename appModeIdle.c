#include "appModeIdle.h"
#include "appMode.h"
#include "lvgl.h"
#include "mifareHal.h"
#include "keyScan.h"

// #define NUM_LETTERS 6

// static const char *abetka_letters[NUM_LETTERS] = {"А", "б", "е", "т", "к", "а"};
// static lv_obj_t *letter_labels[NUM_LETTERS];

// static void anim_fade_cb(void *obj, int32_t v) {
//     lv_obj_set_style_opa((lv_obj_t *) obj, v, 0);  // Cast до lv_obj_t*
// }

// void create_abetka_animation(lv_obj_t *parent) {
//     // Стиль для контейнера з чорним фоном
//     lv_obj_t * canvas = lv_canvas_create(lv_screen_active());
//     lv_canvas_set_buffer(canvas, canvas_buf, 180, 180, render_cf);

//     lv_canvas_fill_bg(canvas, lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 2), LV_OPA_COVER);

//     static lv_style_t style_bg;
//     lv_style_init(&style_bg);
//     lv_style_set_bg_color(&style_bg, LV_COLOR_BLUE);
//     lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);

//     // Стиль для тексту з жовтим кольором
//     static lv_style_t style_text;
//     lv_style_init(&style_text);
//     lv_style_set_text_color(&style_text, lv_color_hex(0xFFFF00));  // Жовтий
//     lv_style_set_text_font(&style_text, &lv_font_ukrainian_48);

//     // Контейнер
//     lv_obj_t *cont = lv_obj_create(parent);
//     lv_obj_set_size(cont, 240, 100);
//     lv_obj_center(cont);
//     lv_obj_add_style(cont, &style_bg, 0);  // Додаємо чорний фон
//     lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
//     lv_obj_set_style_pad_gap(cont, 10, 0);

//     for (int i = 0; i < NUM_LETTERS; i++) {
//         lv_obj_t *label = lv_label_create(cont);
//         lv_label_set_text(label, abetka_letters[i]);
//         lv_obj_add_style(label, &style_text, 0); // Жовтий текст
//         lv_obj_set_style_opa(label, LV_OPA_TRANSP, 0);
//         letter_labels[i] = label;

//         lv_anim_t a;
//         lv_anim_init(&a);
//         lv_anim_set_var(&a, label);
//         lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
//         lv_anim_set_time(&a, 200);
//         lv_anim_set_delay(&a, i * 200);
//         lv_anim_set_exec_cb(&a, anim_fade_cb);
//         lv_anim_start(&a);
//     }
// }

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

static void processInput(Key key, KeyState event)
{
    if (KEY_LEFT == key && KEY_STATE_PRESSED == event) {
        appModeSwitch(APP_MODE_READ_CARD);
    }
}

void appModeIdleEnter()
{
    keyScanInit(processInput);
    lv_disp_set_theme(NULL, lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_YELLOW), false, LV_FONT_DEFAULT));
    // lv_obj_t *scr = lv_scr_act(); // Поточний активний екран
    // create_abetka_animation(scr); // Стартуємо анімацію
    lv_example_hello();
}

void appModeIdleProcess()
{
    // TODO: play animation
}

void appModeIdleExit()
{
    keyScanDeinit();
}
