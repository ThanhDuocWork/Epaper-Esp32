#include "ssd1683.h"
#include "epd_hal.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "epd_gfx.h"

#define EPD_W 400
#define EPD_H 300
#define BUSY_TIMEOUT_MS 20000

#define EPD_INVERT_I1  0

static void set_full_ram_area(void);
static void update_full(void);

void ssd1683_draw_full(const uint8_t *buf)
{
    if (!buf) return;

    set_full_ram_area();

    epd_hal_send_command(0x24); // write RAM (current)
    const int n = (EPD_W * EPD_H / 8);

    for (int i = 0; i < n; i++)
    {
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
    set_full_ram_area();

    // Write previous image (0x26)
    epd_hal_send_command(0x26);  // Write previous image to RAM
    for (int i = 0; i < (EPD_W * EPD_H / 8); i++) {
        epd_hal_send_data(buf_1bpp[i]);
        vTaskDelay(pdMS_TO_TICKS(1));  // Delay after each SPI command
    }

    // Write current image (0x24)
    epd_hal_send_command(0x24);  // Write current image to RAM
    for (int i = 0; i < (EPD_W * EPD_H / 8); i++) {
        epd_hal_send_data(buf_1bpp[i]);
        vTaskDelay(pdMS_TO_TICKS(1));  // Delay after each SPI command
    }

    // Update
    update_full();
}

static const char *TAG = "EPD";

static void wait_busy(uint32_t timeout_ms)
{
    uint32_t t = 0;
    while (epd_hal_busy())
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        t += 10;
        if (t >= timeout_ms)
        {
            ESP_LOGE(TAG, "BUSY timeout");
            break;
        }
    }
}

// full window + set pointer to (0,0)
static void set_full_ram_area(void)
{
    epd_hal_send_command(0x11); // RAM entry mode
    epd_hal_send_data(0x03);    // x inc, y inc

    epd_hal_send_command(0x44); // X range (byte)
    epd_hal_send_data(0x00);
    epd_hal_send_data((EPD_W / 8) - 1);

    epd_hal_send_command(0x45); // Y range (pixel)
    epd_hal_send_data(0x00);
    epd_hal_send_data(0x00);
    epd_hal_send_data((EPD_H - 1) & 0xFF);
    epd_hal_send_data((EPD_H - 1) >> 8);

    epd_hal_send_command(0x4E); // X ptr
    epd_hal_send_data(0x00);

    epd_hal_send_command(0x4F); // Y ptr
    epd_hal_send_data(0x00);
    epd_hal_send_data(0x00);
}

static void write_screen_buffer(uint8_t cmd, uint8_t value)
{
    set_full_ram_area();
    epd_hal_send_command(cmd);
    for (int i = 0; i < (EPD_W * EPD_H / 8); i++)
    {
        epd_hal_send_data(value);
    }
}

static void update_full(void)
{
    epd_hal_send_command(0x21); // Display Update Control
    epd_hal_send_data(0x40);
    epd_hal_send_data(0x00);

    epd_hal_send_command(0x22);
    epd_hal_send_data(0xF7);

    epd_hal_send_command(0x20);
    wait_busy(BUSY_TIMEOUT_MS);
}

// ===== public API =====

void ssd1683_init(void)
{
    epd_hal_init();   // GPIO + SPI 
    epd_hal_reset();

    epd_hal_send_command(0x12); // SWRESET
    wait_busy(BUSY_TIMEOUT_MS);
    epd_hal_send_command(0x01); // Set MUX as 300
    epd_hal_send_data(0x2B);
    epd_hal_send_data(0x01);
    epd_hal_send_data(0x00);

    epd_hal_send_command(0x3C); // BorderWavefrom
    epd_hal_send_data(0x01);

    epd_hal_send_command(0x18); // Read built-in temperature sensor
    epd_hal_send_data(0x80);

    set_full_ram_area();
}

void ssd1683_clear_screen(uint8_t value)
{
    // GxEPD2 clearScreen:
    // _writeScreenBuffer(0x26, value); // previous
    // _writeScreenBuffer(0x24, value); // current
    // refresh(false);
    write_screen_buffer(0x26, value); // previous
    write_screen_buffer(0x24, value); // current
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
    set_full_ram_area();
    epd_hal_send_command(0x24); // current
    for (int i = 0; i < (EPD_W * EPD_H / 8); i++)
    {
        epd_hal_send_data(buf[i]);
    }
    update_full();
}

void ssd1683_draw_demo_box_text(void)
{
    static uint8_t fb[EPD_BUF_SIZE];
    epd_gfx_t g;
    epd_gfx_init(&g, fb);

    // white background
    epd_gfx_clear(&g, true);

    // outer border 
    epd_gfx_rect(&g, 5, 5, EPD_W - 10, EPD_H - 10, false);

    // inner border
    epd_gfx_rect(&g, 20, 40, EPD_W - 40, 120, false);

    // title    
    epd_gfx_print(&g, 30, 15, "GDEY042T81 (BW)\nSSD1683 FULL REFRESH", false, 2);

    // text in box
    epd_gfx_print(&g, 30, 60, "Hello IDF!\nBox + Text OK", false, 2);

    // flush
    ssd1683_write_full(fb);
}
