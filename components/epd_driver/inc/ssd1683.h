#pragma once

#include <stdint.h>

#include "epd_panel.h"

#ifdef __cplusplus
extern "C" {
#endif

void ssd1683_init(void);
void ssd1683_clear_white(void);
void ssd1683_clear_black(void);
void ssd1683_clear_screen(uint8_t value);

void ssd1683_draw_full(const uint8_t *buf);
void ssd1683_draw_1bpp_full(const uint8_t *buf_1bpp);
void ssd1683_draw_1bpp_partial(const uint8_t *buf_1bpp,
                               uint16_t x,
                               uint16_t y,
                               uint16_t w,
                               uint16_t h);
void ssd1683_draw_demo_box_text(void);

#ifdef __cplusplus
}
#endif
