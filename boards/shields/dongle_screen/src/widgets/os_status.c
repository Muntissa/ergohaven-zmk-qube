/*
 * OS indicator widget — shows an Apple or Windows logo depending on whether the
 * "macOS" layer is currently active in the keymap. Follows the manual Win/Mac
 * toggle on the keyboard.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>
#include <lvgl.h>

#include "os_status.h"
#include "fonts.h"

#define OS_GLYPH_MAC "\xEE\x9C\x91" /* U+E711 nf-dev-apple   */
#define OS_GLYPH_WIN "\xEE\x9C\x8F" /* U+E70F nf-dev-windows */

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct os_status_state {
    bool mac;
};

static int last_mac = -1;

static void set_os_symbol(lv_obj_t *label, struct os_status_state state) {
    lv_label_set_text(label, state.mac ? OS_GLYPH_MAC : OS_GLYPH_WIN);
    if ((int)state.mac != last_mac) {
        last_mac = state.mac;
        lv_obj_fade_in(label, 250, 0);
    }
}

static void os_status_update_cb(struct os_status_state state) {
    struct zmk_widget_os_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_os_symbol(widget->obj, state); }
}

static struct os_status_state os_status_get_state(const zmk_event_t *eh) {
    return (struct os_status_state){.mac = zmk_keymap_layer_active(CONFIG_DONGLE_SCREEN_OS_MAC_LAYER)};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_os_status, struct os_status_state, os_status_update_cb,
                            os_status_get_state)
ZMK_SUBSCRIPTION(widget_os_status, zmk_layer_state_changed);

int zmk_widget_os_status_init(struct zmk_widget_os_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);
    lv_obj_set_style_text_font(widget->obj, &nerd_fonts_small, 0);
    lv_label_set_text(widget->obj, OS_GLYPH_WIN);
    sys_slist_append(&widgets, &widget->node);

    widget_os_status_init();
    return 0;
}

lv_obj_t *zmk_widget_os_status_obj(struct zmk_widget_os_status *widget) {
    return widget->obj;
}
