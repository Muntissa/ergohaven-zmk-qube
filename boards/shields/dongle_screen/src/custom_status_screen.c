/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Terminal / dev-HUD status screen (plasma accent).
 */

#include "custom_status_screen.h"

#if CONFIG_DONGLE_SCREEN_OUTPUT_ACTIVE
#include "widgets/output_status.h"
static struct zmk_widget_output_status output_status_widget;
#endif

#if CONFIG_DONGLE_SCREEN_LAYER_ACTIVE
#include "widgets/layer_status.h"
static struct zmk_widget_layer_status layer_status_widget;
#endif

#if CONFIG_DONGLE_SCREEN_BATTERY_ACTIVE
#include "widgets/battery_status.h"
static struct zmk_widget_dongle_battery_status dongle_battery_status_widget;
#endif

#if CONFIG_DONGLE_SCREEN_WPM_ACTIVE
#include "widgets/wpm_status.h"
static struct zmk_widget_wpm_status wpm_status_widget;
#endif

#if CONFIG_DONGLE_SCREEN_MODIFIER_ACTIVE
#include "widgets/mod_status.h"
static struct zmk_widget_mod_status mod_widget;
#endif

#if CONFIG_DONGLE_SCREEN_OS_ACTIVE
#include "widgets/os_status.h"
static struct zmk_widget_os_status os_status_widget;
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "fonts.h"

LV_IMG_DECLARE(eh_robot);

lv_obj_t *screen_splash;
lv_obj_t *screen_main;
lv_style_t global_style;

#define LBL_X 14
#define VAL_X 80
#define ROW0  50
#define ROWP  27

static lv_obj_t *cursor_obj;
static lv_obj_t *scan_obj;

static void on_screen_main_timer(struct k_timer *dummy) { lv_scr_load(screen_main); }
K_TIMER_DEFINE(screen_main_timer, on_screen_main_timer, NULL);

static void cursor_blink_cb(lv_timer_t *t) {
    if (lv_obj_has_flag(cursor_obj, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(cursor_obj, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(cursor_obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static void scan_anim_cb(void *var, int32_t v) { lv_obj_set_y((lv_obj_t *)var, v); }

static lv_obj_t *make_rect(lv_obj_t *parent, int x, int y, int w, int h, lv_color_t color,
                           lv_opa_t opa) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(o, w, h);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_style_bg_color(o, color, 0);
    lv_obj_set_style_bg_opa(o, opa, 0);
    return o;
}

static void make_label(lv_obj_t *parent, const char *txt, int x, int y, lv_color_t color) {
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, color, 0);
    lv_obj_set_pos(l, x, y);
}

lv_obj_t *zmk_display_status_screen() {
    screen_main = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_main, DG_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen_main, 255, LV_PART_MAIN);
    lv_obj_clear_flag(screen_main, LV_OBJ_FLAG_SCROLLABLE);

    lv_style_init(&global_style);
    lv_style_set_text_font(&global_style, &lv_font_unscii_16);
    lv_style_set_text_color(&global_style, DG_FG);
    lv_obj_add_style(screen_main, &global_style, LV_PART_MAIN);

    /* header prompt + blinking cursor */
    make_label(screen_main, "qube@k03:~$ status", LBL_X, 12, DG_ACCENT);
    cursor_obj = make_rect(screen_main, LBL_X + 18 * 8 + 4, 12, 8, 16, DG_ACCENT, 255);
    lv_timer_create(cursor_blink_cb, 530, NULL);

    /* dividers */
    make_rect(screen_main, LBL_X, 36, 252, 1, DG_ACCENT, 110);
    make_rect(screen_main, VAL_X - 12, ROW0 - 2, 1, ROWP * 5 + 18, DG_ACCENT, 70);

    /* row labels */
    make_label(screen_main, "layer", LBL_X, ROW0 + 0 * ROWP, DG_LABEL);
    make_label(screen_main, "mods", LBL_X, ROW0 + 1 * ROWP, DG_LABEL);
    make_label(screen_main, "wpm", LBL_X, ROW0 + 2 * ROWP, DG_LABEL);
    make_label(screen_main, "conn", LBL_X, ROW0 + 3 * ROWP, DG_LABEL);
    make_label(screen_main, "batt", LBL_X, ROW0 + 4 * ROWP, DG_LABEL);
    make_label(screen_main, "os", LBL_X, ROW0 + 5 * ROWP, DG_LABEL);

#if CONFIG_DONGLE_SCREEN_LAYER_ACTIVE
    zmk_widget_layer_status_init(&layer_status_widget, screen_main);
    lv_obj_set_pos(zmk_widget_layer_status_obj(&layer_status_widget), VAL_X, ROW0 + 0 * ROWP);
    lv_obj_set_style_text_color(zmk_widget_layer_status_obj(&layer_status_widget), DG_ACCENT, 0);
#endif

#if CONFIG_DONGLE_SCREEN_MODIFIER_ACTIVE
    zmk_widget_mod_status_init(&mod_widget, screen_main);
    lv_obj_set_pos(zmk_widget_mod_status_obj(&mod_widget), VAL_X, ROW0 + 1 * ROWP);
#endif

#if CONFIG_DONGLE_SCREEN_WPM_ACTIVE
    zmk_widget_wpm_status_init(&wpm_status_widget, screen_main);
    lv_obj_set_pos(zmk_widget_wpm_status_obj(&wpm_status_widget), VAL_X, ROW0 + 2 * ROWP);
#endif

#if CONFIG_DONGLE_SCREEN_OUTPUT_ACTIVE
    zmk_widget_output_status_init(&output_status_widget, screen_main);
    lv_obj_set_pos(zmk_widget_output_status_obj(&output_status_widget), VAL_X, ROW0 + 3 * ROWP);
#endif

#if CONFIG_DONGLE_SCREEN_BATTERY_ACTIVE
    zmk_widget_dongle_battery_status_init(&dongle_battery_status_widget, screen_main);
    lv_obj_set_pos(zmk_widget_dongle_battery_status_obj(&dongle_battery_status_widget), VAL_X,
                   ROW0 + 4 * ROWP);
#endif

#if CONFIG_DONGLE_SCREEN_OS_ACTIVE
    zmk_widget_os_status_init(&os_status_widget, screen_main);
    lv_obj_set_pos(zmk_widget_os_status_obj(&os_status_widget), VAL_X, ROW0 + 5 * ROWP - 4);
    lv_obj_set_style_text_color(zmk_widget_os_status_obj(&os_status_widget), DG_ACCENT, 0);
#endif

    /* sweeping scanline */
    scan_obj = make_rect(screen_main, LBL_X, ROW0 - 4, 252, 2, DG_ACCENT, 80);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, scan_obj);
    lv_anim_set_exec_cb(&a, scan_anim_cb);
    lv_anim_set_values(&a, ROW0 - 4, ROW0 + ROWP * 5 + 12);
    lv_anim_set_time(&a, 3600);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    k_timer_start(&screen_main_timer, K_SECONDS(2), K_NO_WAIT);

    /* boot splash */
    screen_splash = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_splash, DG_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen_splash, 255, LV_PART_MAIN);
    lv_obj_add_style(screen_splash, &global_style, LV_PART_MAIN);

    lv_obj_t *img = lv_img_create(screen_splash);
    lv_img_set_src(img, &eh_robot);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_fade_in(img, 700, 150);

    return screen_splash;
}
