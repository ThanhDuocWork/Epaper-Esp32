#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void epd_lvgl_init(void);
void epd_lvgl_flush_full(void);
void epd_lvgl_fill_screen(int black);
void epd_lvgl_poll_refresh(void);

#ifdef __cplusplus
}
#endif
