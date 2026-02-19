#pragma once
void epd_lvgl_init(void);
#pragma once
#include <stdint.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void epd_lvgl_init(void);                 // init lvgl driver(display)
void epd_lvgl_flush_full(lv_disp_t *disp); // manual flush full screen

#ifdef __cplusplus
}
#endif
