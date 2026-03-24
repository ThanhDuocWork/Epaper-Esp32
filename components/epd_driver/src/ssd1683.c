#include "ssd1683.h"

#include <stdbool.h>
#include <string.h>

#include "epd_gfx.h"
#include "epd_hal.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUSY_TIMEOUT_MS 20000
#define EPD_INVERT_I1 0

static const char *TAG = "EPD";
static bool s_partial_mode_enabled = false;

static void wait_busy(uint32_t timeout_ms);
static void set_ram_area(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
static void set_ram_pointer(uint16_t x, uint16_t y);
static void set_full_ram_area(void);
static void write_screen_buffer(uint8_t cmd, uint8_t value);
static void write_image_buffer(uint8_t cmd, const uint8_t *buf, int len);
static void write_partial_region(uint8_t cmd,
                                 const uint8_t *buf_1bpp,
                                 uint16_t x1,
                                 uint16_t y1,
                                 uint16_t y2,
                                 uint16_t aligned_w);
static void update_full(void);
static void update_partial(void);
static void enter_partial_mode(void);
static void leave_partial_mode(void);
static void ssd1683_write_full(const uint8_t *buf);

void ssd1683_draw_full(const uint8_t *buf)
{
    if (!buf) {
        return;
    }

    leave_partial_mode();
    set_full_ram_area();
    epd_hal_send_command(0x24);

    for (int i = 0; i < EPD_BUF_SIZE; i++) {
#if EPD_INVERT_I1
        epd_hal_send_data((uint8_t)~buf[i]);
#else
        epd_hal_send_data(buf[i]);
#endif
    }

    update_full();
}

void ssd1683_draw_1bpp_full(const uint8_t *buf_1bpp)
{
    if (!buf_1bpp) {
        return;
    }

    leave_partial_mode();
    set_full_ram_area();
    write_image_buffer(0x26, buf_1bpp, EPD_BUF_SIZE);
    set_full_ram_area();
    write_image_buffer(0x24, buf_1bpp, EPD_BUF_SIZE);
    update_full();
}

void ssd1683_draw_1bpp_partial(const uint8_t *buf_1bpp,
                               uint16_t x,
                               uint16_t y,
                               uint16_t w,
                               uint16_t h)
{
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t aligned_w;

    if (!buf_1bpp || w == 0 || h == 0 || x >= EPD_W || y >= EPD_H) {
        return;
    }

    x1 = (uint16_t)(x & ~0x07U);
    y1 = y;
    x2 = (uint16_t)(x + w - 1);
    y2 = (uint16_t)(y + h - 1);

    if (x2 >= EPD_W) {
        x2 = EPD_W - 1;
    }
    if (y2 >= EPD_H) {
        y2 = EPD_H - 1;
    }

    x2 = (uint16_t)((x2 | 0x07U) < EPD_W ? (x2 | 0x07U) : (EPD_W - 1));
    aligned_w = (uint16_t)(x2 - x1 + 1);

    enter_partial_mode();
    set_ram_area(x1, y1, aligned_w, (uint16_t)(y2 - y1 + 1));
    set_ram_pointer(x1, y1);
    write_partial_region(0x24, buf_1bpp, x1, y1, y2, aligned_w);
    update_partial();
}

void ssd1683_draw_1bpp_partial_sync_prev(const uint8_t *buf_1bpp,
                                         uint16_t x,
                                         uint16_t y,
                                         uint16_t w,
                                         uint16_t h)
{
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t aligned_w;

    if (!buf_1bpp || w == 0 || h == 0 || x >= EPD_W || y >= EPD_H) {
        return;
    }

    x1 = (uint16_t)(x & ~0x07U);
    y1 = y;
    x2 = (uint16_t)(x + w - 1);
    y2 = (uint16_t)(y + h - 1);

    if (x2 >= EPD_W) {
        x2 = EPD_W - 1;
    }
    if (y2 >= EPD_H) {
        y2 = EPD_H - 1;
    }

    x2 = (uint16_t)((x2 | 0x07U) < EPD_W ? (x2 | 0x07U) : (EPD_W - 1));
    aligned_w = (uint16_t)(x2 - x1 + 1);

    enter_partial_mode();
    set_ram_area(x1, y1, aligned_w, (uint16_t)(y2 - y1 + 1));
    set_ram_pointer(x1, y1);
    write_partial_region(0x26, buf_1bpp, x1, y1, y2, aligned_w);

    set_ram_area(x1, y1, aligned_w, (uint16_t)(y2 - y1 + 1));
    set_ram_pointer(x1, y1);
    write_partial_region(0x24, buf_1bpp, x1, y1, y2, aligned_w);
    update_partial();
}

static void wait_busy(uint32_t timeout_ms)
{
    uint32_t elapsed_ms = 0;

    while (epd_hal_busy()) {
        vTaskDelay(pdMS_TO_TICKS(10));
        elapsed_ms += 10;

        if (elapsed_ms >= timeout_ms) {
            ESP_LOGE(TAG, "BUSY timeout");
            break;
        }
    }
}

static void set_ram_area(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint16_t x_end = (uint16_t)(x + w - 1);
    uint16_t y_end = (uint16_t)(y + h - 1);

    epd_hal_send_command(0x11);
    epd_hal_send_data(0x03);

    epd_hal_send_command(0x44);
    epd_hal_send_data((x >> 3) & 0xFF);
    epd_hal_send_data((x_end >> 3) & 0xFF);

    epd_hal_send_command(0x45);
    epd_hal_send_data(y & 0xFF);
    epd_hal_send_data((y >> 8) & 0xFF);
    epd_hal_send_data(y_end & 0xFF);
    epd_hal_send_data((y_end >> 8) & 0xFF);
}

static void set_ram_pointer(uint16_t x, uint16_t y)
{
    epd_hal_send_command(0x4E);
    epd_hal_send_data((x >> 3) & 0xFF);

    epd_hal_send_command(0x4F);
    epd_hal_send_data(y & 0xFF);
    epd_hal_send_data((y >> 8) & 0xFF);
}

static void set_full_ram_area(void)
{
    set_ram_area(0, 0, EPD_W, EPD_H);
    set_ram_pointer(0, 0);
}

static void write_screen_buffer(uint8_t cmd, uint8_t value)
{
    set_full_ram_area();
    epd_hal_send_command(cmd);

    for (int i = 0; i < EPD_BUF_SIZE; i++) {
        epd_hal_send_data(value);
    }
}

static void write_image_buffer(uint8_t cmd, const uint8_t *buf, int len)
{
    if (cmd != 0x00) {
        epd_hal_send_command(cmd);
    }

    for (int i = 0; i < len; i++) {
        epd_hal_send_data(buf[i]);
    }
}

static void write_partial_region(uint8_t cmd,
                                 const uint8_t *buf_1bpp,
                                 uint16_t x1,
                                 uint16_t y1,
                                 uint16_t y2,
                                 uint16_t aligned_w)
{
    uint8_t line_buf[EPD_W / 8];

    if (cmd != 0x00) {
        epd_hal_send_command(cmd);
    }

    for (uint16_t row = y1; row <= y2; row++) {
        uint32_t src_index = row * (EPD_W / 8) + (x1 / 8);
        memcpy(line_buf, &buf_1bpp[src_index], aligned_w / 8);
        write_image_buffer(0x00, line_buf, aligned_w / 8);
    }
}

static void update_full(void)
{
    epd_hal_send_command(0x21);
    epd_hal_send_data(0x40);
    epd_hal_send_data(0x00);

    epd_hal_send_command(0x22);
    epd_hal_send_data(0xF7);

    epd_hal_send_command(0x20);
    wait_busy(BUSY_TIMEOUT_MS);
}

static void update_partial(void)
{
    epd_hal_send_command(0x22);
    epd_hal_send_data(0xCF);
    epd_hal_send_command(0x20);
    wait_busy(BUSY_TIMEOUT_MS);
}

static void enter_partial_mode(void)
{
    if (s_partial_mode_enabled) {
        return;
    }

    epd_hal_send_command(0x37);
    epd_hal_send_data(0x00);
    epd_hal_send_data(0x00);
    epd_hal_send_data(0x00);
    epd_hal_send_data(0x00);
    epd_hal_send_data(0x00);
    epd_hal_send_data(0x40);
    epd_hal_send_data(0x00);
    epd_hal_send_data(0x00);
    epd_hal_send_data(0x00);
    epd_hal_send_data(0x00);

    epd_hal_send_command(0x3C);
    epd_hal_send_data(0x80);

    epd_hal_send_command(0x22);
    epd_hal_send_data(0xC0);
    epd_hal_send_command(0x20);
    wait_busy(BUSY_TIMEOUT_MS);

    s_partial_mode_enabled = true;
}

static void leave_partial_mode(void)
{
    if (!s_partial_mode_enabled) {
        return;
    }

    ssd1683_init();
}

void ssd1683_init(void)
{
    epd_hal_init();
    epd_hal_reset();

    epd_hal_send_command(0x12);
    wait_busy(BUSY_TIMEOUT_MS);

    epd_hal_send_command(0x01);
    epd_hal_send_data(0x2B);
    epd_hal_send_data(0x01);
    epd_hal_send_data(0x00);

    epd_hal_send_command(0x3C);
    epd_hal_send_data(0x01);

    epd_hal_send_command(0x18);
    epd_hal_send_data(0x80);

    set_full_ram_area();
    s_partial_mode_enabled = false;
}

void ssd1683_clear_screen(uint8_t value)
{
    leave_partial_mode();
    write_screen_buffer(0x26, value);
    write_screen_buffer(0x24, value);
    update_full();
}

void ssd1683_clear_white(void)
{
    ssd1683_clear_screen(0xFF);
}

void ssd1683_clear_black(void)
{
    ssd1683_clear_screen(0x00);
}

static void ssd1683_write_full(const uint8_t *buf)
{
    if (!buf) {
        return;
    }

    leave_partial_mode();
    set_full_ram_area();
    epd_hal_send_command(0x24);

    for (int i = 0; i < EPD_BUF_SIZE; i++) {
        epd_hal_send_data(buf[i]);
    }

    update_full();
}

void ssd1683_draw_demo_box_text(void)
{
    static uint8_t fb[EPD_BUF_SIZE];
    epd_gfx_t g;

    epd_gfx_init(&g, fb);
    epd_gfx_clear(&g, true);

    epd_gfx_rect(&g, 5, 5, EPD_W - 10, EPD_H - 10, false);
    epd_gfx_rect(&g, 20, 40, EPD_W - 40, 120, false);

    epd_gfx_print(&g, 30, 15, "GDEY042T81 (BW)\nSSD1683 FULL REFRESH", false, 2);
    epd_gfx_print(&g, 30, 60, "Hello IDF!\nBox + Text OK", false, 2);

    ssd1683_write_full(fb);
}
