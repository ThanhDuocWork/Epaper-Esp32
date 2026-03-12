idf_component_register(
    SRCS "EpaperMain.c"
    INCLUDE_DIRS "."
    REQUIRES epd_lvgl_port lvgl
    PRIV_REQUIRES esp_timer
)
