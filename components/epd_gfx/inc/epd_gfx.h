#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "epd_panel.h"

typedef struct {
    uint8_t *buf; /* Size: EPD_BUF_SIZE */
} epd_gfx_t;

void epd_gfx_init(epd_gfx_t *g, uint8_t *buf);
void epd_gfx_clear(epd_gfx_t *g, bool white);

void epd_gfx_pixel(epd_gfx_t *g, int x, int y, bool white);
void epd_gfx_hline(epd_gfx_t *g, int x, int y, int w, bool white);
void epd_gfx_vline(epd_gfx_t *g, int x, int y, int h, bool white);
void epd_gfx_rect(epd_gfx_t *g, int x, int y, int w, int h, bool white);
void epd_gfx_rect_fill(epd_gfx_t *g, int x, int y, int w, int h, bool white);

void epd_gfx_putc(epd_gfx_t *g, int x, int y, char c, bool white, int scale);
void epd_gfx_print(epd_gfx_t *g, int x, int y, const char *s, bool white, int scale);
