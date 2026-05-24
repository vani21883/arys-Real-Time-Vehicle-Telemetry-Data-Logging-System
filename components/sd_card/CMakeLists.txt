idf_component_register(
    SRCS "sd_card.c" "shared_data.c"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_common esp_driver_sdspi esp_driver_sdmmc fatfs esp_timer
)