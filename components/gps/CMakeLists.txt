idf_component_register(
    SRCS "gps.c"
    INCLUDE_DIRS "."
    REQUIRES
        freertos
        esp_common
        esp_driver_uart
        log
        esp_timer
        sd_card
)