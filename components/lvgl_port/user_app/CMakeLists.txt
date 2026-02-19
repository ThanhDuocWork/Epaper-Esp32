idf_component_register(
    SRCS "user_app.cpp" "user_app.c"
    INCLUDE_DIRS "."
    REQUIRES lvgl ui_bsp epaper_driver_bsp
)
# --- IGNORE ---