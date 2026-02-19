#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "epd_lvgl.h"
#include "lvgl.h"

void app_main(void)
{
    ESP_LOGI("MAIN", "LVGL + EPD init");
    epd_lvgl_init();

    // tạo label
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello LVGL on SSD1683!");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);

    // vẽ khung (dùng object + style border)
    lv_obj_t *box = lv_obj_create(lv_screen_active());
    lv_obj_set_size(box, 360, 200);
    lv_obj_align(box, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(box, 2, 0);

    // render full screen 1 lần
    lv_refr_now(NULL);

    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
