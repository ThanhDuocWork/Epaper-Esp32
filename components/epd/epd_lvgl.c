#include "ssd1683.h"
#include "epd_hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "epd_gfx.h"
#include "lvgl.h"

#define EPD_W 400
#define EPD_H 300
#define BUSY_TIMEOUT_MS 20000

#define EPD_INVERT_I1  0

static const char *TAG = "EPD_LVGL";

// Buffer cho ePaper 1bpp (400x300)
static uint8_t s_fb_1bpp[EPD_W * EPD_H / 8];

// LVGL draw buffer (RGB565)
static lv_color_t s_lv_buf[EPD_W * 20]; // 20 line

// Cập nhật pixel trong buffer 1bpp
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

static volatile bool s_need_refresh = false; // Cờ kiểm tra có cần làm mới màn hình không

// LVGL flush callback
static void lv_flush_cb(lv_display_t *display,
                        const lv_area_t *area,
                        uint8_t *px_map)
{
    lv_color_t *cbuf = (lv_color_t *)px_map;

    // Lặp qua các pixel trong khu vực cần làm mới
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            lv_color_t c = *cbuf++;
            uint32_t v = lv_color_to_u32(c);

            uint8_t r = (v >> 16) & 0xFF;
            uint8_t g = (v >> 8) & 0xFF;
            uint8_t b = (v >> 0) & 0xFF;

            uint16_t gray = (r * 30 + g * 59 + b * 11) / 100; // Chuyển sang mức độ xám

            fb_set_px_1bpp(x, y, gray < 128); // Nếu xám < 128, vẽ đen, nếu không vẽ trắng
        }
    }

    // Nếu khu vực là toàn bộ màn hình, đánh dấu cần làm mới
    if (area->x1 == 0 && area->x2 == (EPD_W - 1) && area->y2 == (EPD_H - 1)) {
        s_need_refresh = true;
    }

    lv_display_flush_ready(display); // Đánh dấu hoàn thành
}

// Hàm khởi tạo LVGL
void epd_lvgl_init(void)
{
    // Khởi tạo driver SSD1683
    ssd1683_init();

    // Khởi tạo LVGL core
    lv_init();

    // Khởi tạo driver hiển thị LVGL
    lv_disp_t *s_disp = lv_display_create(EPD_W, EPD_H);

    // Đặt callback flush cho driver LVGL
    lv_display_set_flush_cb(s_disp, lv_flush_cb);

    // Cấu hình bộ đệm vẽ cho LVGL
    lv_display_set_buffers(s_disp,
                           s_lv_buf, NULL,
                           sizeof(s_lv_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    ESP_LOGI(TAG, "LVGL display initialized %dx%d", EPD_W, EPD_H);
}

// Hàm làm đầy màn hình với màu trắng hoặc đen
void epd_lvgl_fill_screen(int black)
{
    // SSD1683: 1 = white, 0 = black
    memset(s_fb_1bpp, black ? 0x00 : 0xFF, sizeof(s_fb_1bpp));
    ESP_LOGI(TAG, "fill_screen: %s", black ? "BLACK" : "WHITE");

    // Làm mới màn hình (full refresh)
    ssd1683_draw_1bpp_full(s_fb_1bpp);
}

// Hàm làm mới toàn bộ màn hình
void epd_lvgl_flush_full(void)
{
    ESP_LOGI(TAG, "flush_full");

    // Gửi dữ liệu vào bộ đệm và làm mới màn hình
    ssd1683_draw_1bpp_full(s_fb_1bpp);
}

// Hàm kiểm tra và làm mới màn hình nếu cần
void epd_lvgl_poll_refresh(void)
{
    if (s_need_refresh) {
        s_need_refresh = false;
        ssd1683_draw_1bpp_full(s_fb_1bpp);   // Làm mới màn hình đúng 1 lần mỗi frame
    }
}
