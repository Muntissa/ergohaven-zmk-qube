#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/hid.h>
#include <zmk/hid_indicators.h>
#include <zmk/keymap.h>
#include <lvgl.h>

#include "mod_status.h"
#include "fonts.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define LED_CLCK 0x02

static void update_mod_status(struct zmk_widget_mod_status *widget)
{
    uint8_t mods = zmk_hid_get_keyboard_report()->body.modifiers;
    bool mac = zmk_keymap_layer_active(CONFIG_DONGLE_SCREEN_OS_MAC_LAYER);
    zmk_hid_indicators_t indicators = zmk_hid_indicators_get_current_profile();

    char text[48] = "";
    int idx = 0;
    if (indicators & LED_CLCK)
        idx += snprintf(&text[idx], sizeof(text) - idx, "[caps] ");
    if (mods & (MOD_LCTL | MOD_RCTL))
        idx += snprintf(&text[idx], sizeof(text) - idx, "[ctrl] ");
    if (mods & (MOD_LSFT | MOD_RSFT))
        idx += snprintf(&text[idx], sizeof(text) - idx, "[shift] ");
    if (mods & (MOD_LALT | MOD_RALT))
        idx += snprintf(&text[idx], sizeof(text) - idx, "[alt] ");
    if (mods & (MOD_LGUI | MOD_RGUI))
        idx += snprintf(&text[idx], sizeof(text) - idx, mac ? "[cmd] " : "[win] ");

    lv_label_set_text(widget->obj, idx ? text : "--");
}

static void mod_status_timer_cb(struct k_timer *timer)
{
    struct zmk_widget_mod_status *widget = k_timer_user_data_get(timer);
    update_mod_status(widget);
}

static struct k_timer mod_status_timer;

int zmk_widget_mod_status_init(struct zmk_widget_mod_status *widget, lv_obj_t *parent)
{
    widget->obj = lv_label_create(parent);
    widget->label = widget->obj;
    lv_label_set_text(widget->obj, "--");

    k_timer_init(&mod_status_timer, mod_status_timer_cb, NULL);
    k_timer_user_data_set(&mod_status_timer, widget);
    k_timer_start(&mod_status_timer, K_MSEC(100), K_MSEC(100));

    return 0;
}

lv_obj_t *zmk_widget_mod_status_obj(struct zmk_widget_mod_status *widget)
{
    return widget->obj;
}
