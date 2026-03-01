#pragma once
void epd_lvgl_init(void);
#pragma once
#include <stdint.h>
#include "lvgl.h"
#include <esp_log.h>
#ifdef __cplusplus
extern "C" {
#endif

void epd_lvgl_init(void);                 // init lvgl driver(display)
void epd_lvgl_flush_full(void); // manual flush full screen
void epd_lvgl_fill_screen(int black); // black=1 => full black, black=0 => full white
void epd_lvgl_poll_refresh(void); 
#ifdef __cplusplus
}
#endif
