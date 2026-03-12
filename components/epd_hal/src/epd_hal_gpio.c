#include "epd_hal.h"
#include "epd_pins.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

void epd_hal_gpio_init(void)
{
    gpio_config_t io = {0};

    // BUSY input + pull-up
    io.pin_bit_mask = 1ULL << EPD_PIN_BUSY;
    io.mode = GPIO_MODE_INPUT;
    io.pull_up_en = GPIO_PULLUP_ENABLE;     
    io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io);

    // DC + RST + CS output
    io.pin_bit_mask = (1ULL << EPD_PIN_DC) | (1ULL << EPD_PIN_RES) | (1ULL << EPD_PIN_CS);
    io.mode = GPIO_MODE_OUTPUT;
    io.pull_up_en = GPIO_PULLUP_DISABLE;
    io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io);

    gpio_set_level(EPD_PIN_CS, 1);   
    gpio_set_level(EPD_PIN_DC, 1);
    gpio_set_level(EPD_PIN_RES, 1);
}


void epd_hal_reset(void)
{
    gpio_set_level(EPD_PIN_RES, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(EPD_PIN_RES, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void epd_hal_delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

bool epd_hal_busy(void)
{
    return gpio_get_level(EPD_PIN_BUSY) == 1;
}
