#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "epd_lvgl.h"
#include "lvgl.h"
#include "ui.h"

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

static void ui_demo_init(void)
{
    ui_set_status("Booting");
    ui_set_temperature(24);
    ui_set_battery(100);
}

static void ui_demo_step(int step)
{
    char status[32];

    snprintf(status, sizeof(status), "Tick %d", step);
    ui_set_status(status);
}

static void lv_task(void *arg)
{
    TickType_t last_update_tick;
    int step = 0;

    (void)arg;

    epd_lvgl_init();
    lv_tick_timer_init();
    ui_init();
    ui_demo_init();

    lv_timer_handler();
    epd_lvgl_flush_full();

    last_update_tick = xTaskGetTickCount();

    while (1) {
        lv_timer_handler();
        epd_lvgl_poll_refresh();

        if ((xTaskGetTickCount() - last_update_tick) >= pdMS_TO_TICKS(6000)) {
            last_update_tick = xTaskGetTickCount();
            step++;
            ui_demo_step(step);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void app_main(void)
{
    xTaskCreatePinnedToCore(lv_task, "lv_task", 8192, NULL, 5, NULL, 0);
}
