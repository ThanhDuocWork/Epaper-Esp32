#include "epd_pins.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

static spi_device_handle_t s_epd_spi = NULL;
static const char *TAG = "EPD";

void epd_hal_spi_init(void)
{
    if (s_epd_spi) {
        return;
    }

    spi_bus_config_t buscfg = {
        .sclk_io_num = EPD_PIN_SCL,
        .mosi_io_num = EPD_PIN_SDA,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 4 * 1000 * 1000,
        .mode = 0,
        /* CS is toggled manually in epd_hal_send_command/data. */
        .spics_io_num = -1,
        .queue_size = 1,
    };

    esp_err_t err = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_LOGI(TAG, "spi_bus_initialize = %s", esp_err_to_name(err));
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return;
    }

    err = spi_bus_add_device(SPI2_HOST, &devcfg, &s_epd_spi);
    ESP_LOGI(TAG, "spi_bus_add_device = %s, handle=%p", esp_err_to_name(err), s_epd_spi);
    if (err != ESP_OK) {
        s_epd_spi = NULL;
    }
}

void epd_hal_spi_write(const uint8_t *data, int len)
{
    if (s_epd_spi == NULL)
    {
        ESP_LOGE(TAG, "SPI device not initialized");
        return;
    }

    spi_transaction_t t = {0};
    t.length = len * 8;
    t.tx_buffer = data;

    ESP_ERROR_CHECK(spi_device_transmit(s_epd_spi, &t));
}
