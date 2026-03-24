#include "epd_lvgl.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "epd_gfx.h"
#include "epd_hal.h"
#include "ssd1683.h"
#include "esp_log.h"

#define EPD_MAX_DIRTY_RECTS 2

static const char *TAG = "EPD_LVGL";

static uint8_t s_fb_1bpp[EPD_BUF_SIZE];
static uint16_t s_lv_buf[EPD_W * 20];
static volatile bool s_refresh_pending = false;
static bool s_dirty_overflow = false;
static lv_area_t s_dirty_rects[EPD_MAX_DIRTY_RECTS];
static uint32_t s_dirty_count = 0;

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

static bool areas_overlap_or_touch(const lv_area_t *a, const lv_area_t *b)
{
    return !(a->x2 < (b->x1 - 1) ||
             b->x2 < (a->x1 - 1) ||
             a->y2 < (b->y1 - 1) ||
             b->y2 < (a->y1 - 1));
}

static void area_merge(lv_area_t *dst, const lv_area_t *src)
{
    if (src->x1 < dst->x1) {
        dst->x1 = src->x1;
    }
    if (src->y1 < dst->y1) {
        dst->y1 = src->y1;
    }
    if (src->x2 > dst->x2) {
        dst->x2 = src->x2;
    }
    if (src->y2 > dst->y2) {
        dst->y2 = src->y2;
    }
}

static void dirty_rects_reset(void)
{
    s_dirty_count = 0;
    s_dirty_overflow = false;
    memset(s_dirty_rects, 0, sizeof(s_dirty_rects));
}

static void log_rect(const char *label, const lv_area_t *area, uint32_t index)
{
    ESP_LOGI(TAG,
             "%s[%lu] x1=%d y1=%d x2=%d y2=%d w=%d h=%d",
             label,
             (unsigned long)index,
             area->x1,
             area->y1,
             area->x2,
             area->y2,
             area->x2 - area->x1 + 1,
             area->y2 - area->y1 + 1);
}

static void dirty_rects_add(const lv_area_t *area)
{
    uint32_t i;

    for (i = 0; i < s_dirty_count; i++) {
        if (areas_overlap_or_touch(&s_dirty_rects[i], area)) {
            area_merge(&s_dirty_rects[i], area);
            log_rect("dirty merge", &s_dirty_rects[i], i);
            return;
        }
    }

    if (s_dirty_count < EPD_MAX_DIRTY_RECTS) {
        s_dirty_rects[s_dirty_count] = *area;
        log_rect("dirty rect", &s_dirty_rects[s_dirty_count], s_dirty_count);
        s_dirty_count++;
        return;
    }

    s_dirty_overflow = true;
    ESP_LOGW(TAG, "dirty rect overflow, falling back to full refresh");
}

static void lv_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    uint16_t *color_buf = (uint16_t *)px_map;

    ESP_LOGI(TAG,
             "lvgl flush x1=%d y1=%d x2=%d y2=%d w=%d h=%d",
             area->x1,
             area->y1,
             area->x2,
             area->y2,
             area->x2 - area->x1 + 1,
             area->y2 - area->y1 + 1);

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

    dirty_rects_add(area);
    s_refresh_pending = true;
    lv_display_flush_ready(display);
}

void epd_lvgl_init(void)
{
    lv_display_t *disp;

    ssd1683_init();
    lv_init();

    memset(s_fb_1bpp, 0xFF, sizeof(s_fb_1bpp));
    dirty_rects_reset();

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

    if (s_dirty_overflow || s_dirty_count == 0) {
        ESP_LOGI(TAG, "full refresh fallback");
        ssd1683_draw_1bpp_full(s_fb_1bpp);
        dirty_rects_reset();
        return;
    }

    if (s_dirty_count == 1) {
        uint16_t width = (uint16_t)(s_dirty_rects[0].x2 - s_dirty_rects[0].x1 + 1);
        uint16_t height = (uint16_t)(s_dirty_rects[0].y2 - s_dirty_rects[0].y1 + 1);

        log_rect("partial rect", &s_dirty_rects[0], 0);
        ssd1683_draw_1bpp_partial(s_fb_1bpp,
                                  (uint16_t)s_dirty_rects[0].x1,
                                  (uint16_t)s_dirty_rects[0].y1,
                                  width,
                                  height);
        dirty_rects_reset();
        return;
    }

    if (s_dirty_count == 2) {
        uint16_t old_w = (uint16_t)(s_dirty_rects[0].x2 - s_dirty_rects[0].x1 + 1);
        uint16_t old_h = (uint16_t)(s_dirty_rects[0].y2 - s_dirty_rects[0].y1 + 1);
        uint16_t new_w = (uint16_t)(s_dirty_rects[1].x2 - s_dirty_rects[1].x1 + 1);
        uint16_t new_h = (uint16_t)(s_dirty_rects[1].y2 - s_dirty_rects[1].y1 + 1);

        log_rect("old rect", &s_dirty_rects[0], 0);
        log_rect("new rect", &s_dirty_rects[1], 1);

        ESP_LOGI(TAG, "old rect uses sync-prev partial");
        ssd1683_draw_1bpp_partial_sync_prev(s_fb_1bpp,
                                            (uint16_t)s_dirty_rects[0].x1,
                                            (uint16_t)s_dirty_rects[0].y1,
                                            old_w,
                                            old_h);

        ESP_LOGI(TAG, "new rect uses current-only partial");
        ssd1683_draw_1bpp_partial(s_fb_1bpp,
                                  (uint16_t)s_dirty_rects[1].x1,
                                  (uint16_t)s_dirty_rects[1].y1,
                                  new_w,
                                  new_h);

        dirty_rects_reset();
        return;
    }

    ESP_LOGI(TAG, "full refresh fallback");
    ssd1683_draw_1bpp_full(s_fb_1bpp);
    dirty_rects_reset();
}
