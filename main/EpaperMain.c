#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1683.h"

void app_main(void)
{
    ESP_LOGI("MAIN", "EPD init...");
    ssd1683_init();

    ESP_LOGI("MAIN", "Demo box+text");
    ssd1683_draw_demo_box_text();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
