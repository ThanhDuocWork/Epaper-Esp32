#pragma once
#include <stdint.h>

void ssd1683_init(void);
void ssd1683_clear_white(void);
void ssd1683_clear_black(void);
void ssd1683_clear_screen(uint8_t value); 
void ssd1683_draw_demo_box_text(void);

void ssd1683_draw_full(const uint8_t *buf);

void ssd1683_draw_1bpp_full(const uint8_t *buf_1bpp); // buf size = 400*300/8
