#pragma once

#include "lvgl.h"
#include <esp_log.h>
#include "ssd1683.h"
#include "epd_hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "epd_gfx.h"
#include "lvgl.h"

#define EPD_INVERT_I1 0
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
