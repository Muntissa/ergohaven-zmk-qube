/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>

#include "wpm_status.h"
#include <fonts.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct wpm_status_state {
    int wpm;
};

static struct wpm_status_state get_state(const zmk_event_t *_eh) {
    const struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(_eh);
    return (struct wpm_status_state){.wpm = ev ? ev->state : 0};
}

static void set_wpm(struct zmk_widget_wpm_status *widget, struct wpm_status_state state) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%i", state.wpm < 0 ? 0 : state.wpm);
    lv_label_set_text(widget->obj, buf);
}

static void wpm_status_update_cb(struct wpm_status_state state) {
    struct zmk_widget_wpm_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_wpm(widget, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_wpm_status, struct wpm_status_state, wpm_status_update_cb,
                            get_state)
ZMK_SUBSCRIPTION(widget_wpm_status, zmk_wpm_state_changed);

int zmk_widget_wpm_status_init(struct zmk_widget_wpm_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);
    widget->wpm_label = widget->obj;
    lv_label_set_text(widget->obj, "0");
    sys_slist_append(&widgets, &widget->node);
    widget_wpm_status_init();
    return 0;
}

lv_obj_t *zmk_widget_wpm_status_obj(struct zmk_widget_wpm_status *widget) {
    return widget->obj;
}
