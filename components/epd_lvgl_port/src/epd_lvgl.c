#include <stdint.h>
#include <epd_lvgl.h>


#define EPD_W 400
#define EPD_H 300
#define BUSY_TIMEOUT_MS 20000
static const char *TAG = "EPD_LVGL";

// Buffer ePaper 1bpp (400x300)
static uint8_t s_fb_1bpp[EPD_W * EPD_H / 8];

// LVGL draw buffer (RGB565)
static lv_color_t s_lv_buf[EPD_W * 20]; // 20 line

// Update pixel buffer 1bpp
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

static volatile bool flag_refresh = false;

// LVGL flush callback
static void lv_flush_cb(lv_display_t *display,
                        const lv_area_t *area,
                        uint8_t *px_map)
{
    lv_color_t *cbuf = (lv_color_t *)px_map;

    // Lặp qua các pixel trong khu vực cần làm mới
    for (int y = area->y1; y <= area->y2; y++)
    {
        for (int x = area->x1; x <= area->x2; x++)
        {
            lv_color_t c = *cbuf++;
            uint32_t v = lv_color_to_u32(c);

            uint8_t r = (v >> 16) & 0xFF;
            uint8_t g = (v >> 8) & 0xFF;
            uint8_t b = (v >> 0) & 0xFF;

            uint16_t gray = (r * 30 + g * 59 + b * 11) / 100; // convert RGB565 to grayscale

            fb_set_px_1bpp(x, y, gray < 128); // if grayscale < 128, draw black, if >= 128, draw white
        }
    }
    if (area->x1 == 0 &&
        area->y1 == 0 &&
        area->x2 == (EPD_W - 1) &&
        area->y2 == (EPD_H - 1))
    {
        flag_refresh = true;
    }

    lv_display_flush_ready(display); 
}

void epd_lvgl_init(void)
{
    ssd1683_init();
    lv_init();
    // clear buffer 1bpp (white)
    memset(s_fb_1bpp, 0xFF, sizeof(s_fb_1bpp));
    // initial driver LVGL
    lv_disp_t *s_disp = lv_display_create(EPD_W, EPD_H);

    // callback flush  driver LVGL
    lv_display_set_flush_cb(s_disp, lv_flush_cb);

    // config buffer drawing LVGL
    lv_display_set_buffers(s_disp,
                           s_lv_buf, NULL,
                           sizeof(s_lv_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    ESP_LOGI(TAG, "LVGL display initialized %dx%d", EPD_W, EPD_H);
}

void epd_lvgl_fill_screen(int black)
{
    // SSD1683: 1 = white, 0 = black
    memset(s_fb_1bpp, black ? 0x00 : 0xFF, sizeof(s_fb_1bpp));
    ESP_LOGI(TAG, "fill_screen: %s", black ? "BLACK" : "WHITE");

    // refresh screen (full refresh)
    ssd1683_draw_1bpp_full(s_fb_1bpp);
}

void epd_lvgl_flush_full(void)
{
    ESP_LOGI(TAG, "flush_full");
    ssd1683_draw_1bpp_full(s_fb_1bpp);
}
void epd_lvgl_poll_refresh(void)
{
    if (!flag_refresh)
        return;

    if (epd_hal_busy())
        return;

    flag_refresh = false;

    ssd1683_draw_1bpp_full(s_fb_1bpp);
}
