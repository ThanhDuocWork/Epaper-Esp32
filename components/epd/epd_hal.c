#include "epd_hal.h"
#include "epd_pins.h"

// from spi hal
void epd_hal_spi_init(void);
void epd_hal_spi_write(const uint8_t *data, int len);

void epd_hal_send_command(uint8_t cmd)
{
    gpio_set_level(EPD_PIN_DC, 0);
    gpio_set_level(EPD_PIN_CS, 0);
    epd_hal_spi_write(&cmd, 1);
    gpio_set_level(EPD_PIN_CS, 1);
}

void epd_hal_send_data(uint8_t data)
{
    gpio_set_level(EPD_PIN_DC, 1);
    gpio_set_level(EPD_PIN_CS, 0);
    epd_hal_spi_write(&data, 1);
    gpio_set_level(EPD_PIN_CS, 1);
}

void epd_hal_init(void)
{
    epd_hal_gpio_init();    
    epd_hal_spi_init();
}