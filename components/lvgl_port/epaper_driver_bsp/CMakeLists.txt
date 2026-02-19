idf_component_register(
    SRCS
        "epaper_driver_bsp.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        lvgl
        epd
)
# --- IGNORE ---