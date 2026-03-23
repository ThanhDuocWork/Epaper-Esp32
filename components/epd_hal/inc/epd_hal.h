#pragma once

#include <stdbool.h>
#include <stdint.h>

void epd_hal_init(void);
void epd_hal_reset(void);
void epd_hal_delay_ms(uint32_t ms);

void epd_hal_send_command(uint8_t cmd);
void epd_hal_send_data(uint8_t data);

bool epd_hal_busy(void);

void epd_hal_spi_init(void);
void epd_hal_gpio_init(void);
