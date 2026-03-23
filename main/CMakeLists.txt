idf_component_register(
    SRCS "EpaperMain.c"
    INCLUDE_DIRS "."
    REQUIRES app_ui epd_lvgl_port
    PRIV_REQUIRES esp_timer
)
