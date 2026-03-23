#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void ui_set_status(const char *text);
void ui_set_temperature(int value_c);
void ui_set_battery(int percent);

#ifdef __cplusplus
}
#endif
