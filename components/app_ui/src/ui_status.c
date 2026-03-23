#include "ui_status.h"

#include "ui_internal.h"

void ui_set_status(const char *text)
{
    if (!g_ui_label_status || !text) {
        return;
    }

    lv_label_set_text_fmt(g_ui_label_status, "Status: %s", text);
}

void ui_set_temperature(int value_c)
{
    if (!g_ui_label_temp) {
        return;
    }

    lv_label_set_text_fmt(g_ui_label_temp, "Temp: %d C", value_c);
}

void ui_set_battery(int percent)
{
    if (!g_ui_label_battery) {
        return;
    }

    if (percent < 0) {
        percent = 0;
    }
    if (percent > 100) {
        percent = 100;
    }

    lv_label_set_text_fmt(g_ui_label_battery, "Battery: %d%%", percent);
}
