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

static void lv_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    // px_map buffer RGB565 (lv_color_t)
    lv_color_t *cbuf = (lv_color_t *)px_map;

    for (int y = area->y1; y <= area->y2; y++)
    {
        for (int x = area->x1; x <= area->x2; x++)
        {

            // convert RGB565 -> grayscale
            // lv_color_t on v9 is 16-bit 565
            lv_color_t c = *cbuf++;
            uint32_t v = lv_color_to_u32(c); // ra 0xRRGGBB

            uint8_t r = (v >> 16) & 0xFF;
            uint8_t g = (v >> 8) & 0xFF;
            uint8_t b = (v >> 0) & 0xFF;

            // scale to 0..255
            uint8_t rr = (r * 255) / 31;
            uint8_t gg = (g * 255) / 63;
            uint8_t bb = (b * 255) / 31;

            // luminance
            uint16_t gray = (rr * 30 + gg * 59 + bb * 11) / 100;

            // threshold: <128 => black
            fb_set_px_1bpp(x, y, gray < 128);
        }
    }
    // full refresh only: After each flush, full refresh the entire area immediately.
    // But LVGL flushes multiple times -> it will slow down.
    // => Strategy: Only refresh when the area is full screen (simple)
    if (area->x1 == 0 && area->y1 == 0 && area->x2 == (EPD_W - 1) && area->y2 == (EPD_H - 1))
    {
        // push s_fb_1bpp to epaper
        ssd1683_draw_1bpp_full(s_fb_1bpp);
        ESP_LOGI(TAG, "Full flush -> refresh");
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
