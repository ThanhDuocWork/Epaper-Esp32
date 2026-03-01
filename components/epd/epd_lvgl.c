#include "epd_lvgl.h"
#include "ssd1683.h"
#include "esp_log.h"

#define EPD_W 400
#define EPD_H 300

static const char *TAG = "EPD_LVGL";

static lv_disp_t *s_disp = NULL;
static lv_display_t *s_display = NULL;

// buffer 1bpp cho epaper: 400*300/8 = 15000 bytes
static uint8_t s_fb_1bpp[EPD_W * EPD_H / 8];

// LVGL draw buffer (RGB565) - Little RAM
static lv_color_t s_lv_buf[EPD_W * 20]; // 20 line

static inline void fb_set_px_1bpp(uint16_t x, uint16_t y, int is_black)
{
    uint32_t i = y * (EPD_W / 8) + (x / 8);
    uint8_t m = 0x80 >> (x & 7);

    // SSD1683: 1 = white, 0 = black
    if (is_black)
        s_fb_1bpp[i] &= ~m;
    else
        s_fb_1bpp[i] |= m;
}
static volatile bool s_need_refresh = false;
static void lv_flush_cb(lv_display_t *display,
                        const lv_area_t *area,
                        uint8_t *px_map)
{
    lv_color_t *cbuf = (lv_color_t *)px_map;

    for (int y = area->y1; y <= area->y2; y++)
    {
        for (int x = area->x1; x <= area->x2; x++)
        {
            lv_color_t c = *cbuf++;
            uint32_t v = lv_color_to_u32(c);

            uint8_t r = (v >> 16) & 0xFF;
            uint8_t g = (v >> 8) & 0xFF;
            uint8_t b = (v >> 0) & 0xFF;

            uint16_t gray = (r * 30 + g * 59 + b * 11) / 100;

            fb_set_px_1bpp(x, y, gray < 128);
        }
    }
    if (area->x1 == 0 && area->x2 == (EPD_W - 1) && area->y2 == (EPD_H - 1)) {
        s_need_refresh = true;
    }

    lv_display_flush_ready(display);
}

void epd_lvgl_init(void)
{
    // init panel driver
    ssd1683_init();

    // init lvgl core s
    lv_init();

    // display driver
    s_display = lv_display_create(EPD_W, EPD_H);

    lv_display_set_flush_cb(s_display, lv_flush_cb);

    // set draw buffer
    lv_display_set_buffers(
        s_display,
        s_lv_buf, NULL,
        sizeof(s_lv_buf),
        LV_DISPLAY_RENDER_MODE_PARTIAL);

    ESP_LOGI(TAG, "LVGL display inited %dx%d", EPD_W, EPD_H);
}
#include <string.h>
#include "esp_log.h"

void epd_lvgl_fill_screen(int black)
{
    // SSD1683: 1 = white, 0 = black
    memset(s_fb_1bpp, black ? 0x00 : 0xFF, sizeof(s_fb_1bpp));
    ESP_LOGI(TAG, "fill_screen: %s", black ? "BLACK" : "WHITE");
    // Full refresh 
    ssd1683_draw_1bpp_full(s_fb_1bpp);
}
void epd_lvgl_flush_full(void)
{
    ssd1683_draw_1bpp_full(s_fb_1bpp);
}
void epd_lvgl_poll_refresh(void)
{
    if (s_need_refresh) {
        s_need_refresh = false;
        ssd1683_draw_1bpp_full(s_fb_1bpp);   
}