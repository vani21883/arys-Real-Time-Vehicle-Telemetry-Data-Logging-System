idf_component_register(
    SRCS "rpm.c"
    INCLUDE_DIRS "."
    REQUIRES
        freertos
        esp_common
        esp_driver_gpio
        esp_driver_pcnt
        esp_timer
        sd_card
)