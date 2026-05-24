idf_component_register(
    SRCS "can.c"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_common driver sd_card esp_timer
)