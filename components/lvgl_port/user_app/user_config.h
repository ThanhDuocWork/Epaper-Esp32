#ifndef USER_CONFIG_H
#define USER_CONFIG_H
#pragma once

// ====== E-Paper panel size (đổi đúng theo màn của bạn) ======
#define EPD_WIDTH     400
#define EPD_HEIGHT    300

// ====== SPI host (ESP32-S3: SPI2_HOST hoặc SPI3_HOST tuỳ bạn dùng) ======
#define EPD_SPI_NUM   SPI2_HOST

// ====== EPD control pins ======
#define EPD_CS_PIN    10
#define EPD_DC_PIN    9
#define EPD_RST_PIN   8
#define EPD_BUSY_PIN  7
#define EPD_MOSI_PIN  11
#define EPD_SCK_PIN   12
#define EPD_PWR_PIN    -1
#define Audio_PWR_PIN  -1
#define VBAT_PWR_PIN   -1

#endif