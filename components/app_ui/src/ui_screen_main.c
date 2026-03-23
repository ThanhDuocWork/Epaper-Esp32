#include "ui_screen_main.h"

#include "lvgl.h"
#include "ui_internal.h"

lv_obj_t *g_ui_label_status;
lv_obj_t *g_ui_label_temp;
lv_obj_t *g_ui_label_battery;

static void style_screen(void)
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
}

static lv_obj_t *create_panel(lv_obj_t *parent, int32_t width, int32_t height)
{
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_remove_style_all(panel);
    lv_obj_set_size(panel, width, height);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, lv_color_black(), 0);
    lv_obj_set_style_border_width(panel, 2, 0);
    lv_obj_set_style_pad_all(panel, 12, 0);
    return panel;
}

static void create_title(void)
{
    lv_obj_t *title = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_label_set_text(title, "E-Paper Partial Refresh Test");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 16, 12);
}

static void create_status_panel(void)
{
    lv_obj_t *panel = create_panel(lv_screen_active(), 180, 110);
    lv_obj_set_pos(panel, 16, 48);

    g_ui_label_status = lv_label_create(panel);
    lv_obj_set_width(g_ui_label_status, 150);
    lv_obj_set_style_text_color(g_ui_label_status, lv_color_black(), 0);
    lv_label_set_long_mode(g_ui_label_status, LV_LABEL_LONG_WRAP);
    lv_label_set_text(g_ui_label_status, "Status: Booting");
    lv_obj_set_pos(g_ui_label_status, 0, 0);

    g_ui_label_temp = lv_label_create(panel);
    lv_obj_set_style_text_color(g_ui_label_temp, lv_color_black(), 0);
    lv_label_set_text(g_ui_label_temp, "Temp: 24 C");
    lv_obj_set_pos(g_ui_label_temp, 0, 34);

    g_ui_label_battery = lv_label_create(panel);
    lv_obj_set_style_text_color(g_ui_label_battery, lv_color_black(), 0);
    lv_label_set_text(g_ui_label_battery, "Battery: 100%");
    lv_obj_set_pos(g_ui_label_battery, 0, 68);
}

void ui_init(void)
{
    style_screen();
    create_title();
    create_status_panel();
}
