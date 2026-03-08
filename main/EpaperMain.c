#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "epd_lvgl.h"
#include "ssd1683.h"

static esp_timer_handle_t s_lv_tick_timer;

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
    ESP_ERROR_CHECK(esp_timer_start_periodic(s_lv_tick_timer, 1000));
}

static void lv_task(void *arg)
{
    while (1) {
        epd_lvgl_fill_screen(0); 
        vTaskDelay(pdMS_TO_TICKS(5000)); 

        epd_lvgl_fill_screen(1); 
        vTaskDelay(pdMS_TO_TICKS(5000)); 
    }
}

void app_main(void)
{
    epd_lvgl_init();
    lv_tick_timer_init(); 
    xTaskCreatePinnedToCore(lv_task, "lv_task", 4096, NULL, 5, NULL, 0);
    ESP_LOGI("MAIN", "lv_task started");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000)); 
    }
}