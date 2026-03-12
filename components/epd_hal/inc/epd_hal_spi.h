#pragma once
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

typedef struct {
    spi_device_handle_t spi;
    gpio_num_t cs;
} epd_spi_t;

esp_err_t epd_spi_init(
    epd_spi_t *bus,
    int pin_sck,
    int pin_mosi,
    int pin_cs,
    int clock_hz
);

void epd_spi_cs_low(epd_spi_t *bus);
void epd_spi_cs_high(epd_spi_t *bus);

esp_err_t epd_spi_write(epd_spi_t *bus, const uint8_t *data, int len);
