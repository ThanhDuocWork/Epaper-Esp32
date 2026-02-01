idf_component_register(SRCS "EpaperMain.c"
                       PRIV_REQUIRES spi_flash
                    INCLUDE_DIRS "."
                    REQUIRES epd)
