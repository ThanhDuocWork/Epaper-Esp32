idf_component_register(
    SRCS
        "epd_hal.c"
        "epd_hal_gpio.c"
        "epd_hal_spi.c"
        "ssd1683.c"
        "epd_gfx.c"
        "epd_lvgl.c"
    INCLUDE_DIRS "include"
    REQUIRES driver freertos log lvgl
)

