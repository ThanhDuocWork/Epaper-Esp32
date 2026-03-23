#include "epd_lvgl.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "epd_gfx.h"
#include "epd_hal.h"
#include "ssd1683.h"
#include "esp_log.h"

static const char *TAG = "EPD_LVGL";

static uint8_t s_fb_1bpp[EPD_BUF_SIZE];
static uint16_t s_lv_buf[EPD_W * 20];
static volatile bool s_refresh_pending = false;

static inline void fb_set_px_1bpp(uint16_t x, uint16_t y, bool is_black)
{
    uint32_t index = y * (EPD_W / 8) + (x / 8);
    uint8_t mask = 0x80 >> (x & 7);

    if (is_black) {
        s_fb_1bpp[index] &= (uint8_t)~mask;
    }
    else {
        s_fb_1bpp[index] |= mask;
    }
}

static void lv_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    uint16_t *color_buf = (uint16_t *)px_map;

    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            uint16_t color = *color_buf++;
            uint8_t r = (uint8_t)((((color >> 11) & 0x1F) * 255) / 31);
            uint8_t g = (uint8_t)((((color >> 5) & 0x3F) * 255) / 63);
            uint8_t b = (uint8_t)(((color & 0x1F) * 255) / 31);
            uint16_t gray = (uint16_t)((r * 30 + g * 59 + b * 11) / 100);

            fb_set_px_1bpp((uint16_t)x, (uint16_t)y, gray < 128);
        }
    }

    s_refresh_pending = true;
    lv_display_flush_ready(display);
}

void epd_lvgl_init(void)
{
    lv_display_t *disp;

    ssd1683_init();
    lv_init();

    memset(s_fb_1bpp, 0xFF, sizeof(s_fb_1bpp));

    ssd1683_clear_white();
    ssd1683_draw_1bpp_full(s_fb_1bpp);

    disp = lv_display_create(EPD_W, EPD_H);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, lv_flush_cb);
    lv_display_set_buffers(disp,
                           s_lv_buf,
                           NULL,
                           sizeof(s_lv_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    ESP_LOGI(TAG, "LVGL display initialized %dx%d", EPD_W, EPD_H);
}

void epd_lvgl_fill_screen(int black)
{
    memset(s_fb_1bpp, black ? 0x00 : 0xFF, sizeof(s_fb_1bpp));
    ssd1683_draw_1bpp_full(s_fb_1bpp);
}

void epd_lvgl_flush_full(void)
{
    ssd1683_draw_1bpp_full(s_fb_1bpp);
}

void epd_lvgl_poll_refresh(void)
{
    if (!s_refresh_pending) {
        return;
    }

    if (epd_hal_busy()) {
        return;
    }

    s_refresh_pending = false;
    ESP_LOGI(TAG, "full refresh");
    ssd1683_draw_1bpp_full(s_fb_1bpp);
}
