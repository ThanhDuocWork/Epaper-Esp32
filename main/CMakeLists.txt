idf_component_register(
    SRCS "EpaperMain.c"
    INCLUDE_DIRS "."
    REQUIRES epd lvgl lvgl_port
    PRIV_REQUIRES epd lvgl esp_timer
)