#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "lvgl.h"
#include "epd_lvgl.h"

static const char *TAG = "MAIN";
static esp_timer_handle_t s_lv_tick_timer;

// 1ms tick for LVGL
static void lv_tick_cb(void *arg)
{
    (void)arg;
    lv_tick_inc(1);
}

static void lv_tick_timer_init(void)
{
    const esp_timer_create_args_t args = {
        .callback = &lv_tick_cb,
        .name = "lv_tick"
    };

    ESP_ERROR_CHECK(esp_timer_create(&args, &s_lv_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(s_lv_tick_timer, 1000)); // 1ms
}

// set full screen color
static void lv_set_full_color(bool black)
{
    lv_obj_t *scr = lv_screen_active();
    lv_color_t c = black ? lv_color_black() : lv_color_white();

    lv_obj_set_style_bg_color(scr, c, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_invalidate(scr);
}

// LVGL task
static void lv_task(void *arg)
{
    (void)arg;

    bool black = false;

    while (1)
    {
        ESP_LOGI(TAG, "Setting screen to %s", black ? "BLACK" : "WHITE");

        lv_set_full_color(black);
        lv_refr_now(NULL);
        epd_lvgl_poll_refresh();
        // Wait a bit to ensure the refresh is done before changing the screen again (fix next commit)
        for (int i = 0; i < 500; i++) {
            lv_timer_handler();
            epd_lvgl_poll_refresh();
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        black = !black; // đảo trạng thái
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "boot");

    epd_lvgl_init();
    lv_tick_timer_init();

    xTaskCreatePinnedToCore(lv_task, "lv", 4096, NULL, 5, NULL, 0);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}