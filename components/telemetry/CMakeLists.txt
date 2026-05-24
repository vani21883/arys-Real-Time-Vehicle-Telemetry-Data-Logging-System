idf_component_register(
    SRCS "telemetry.c"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_common esp_wifi esp_netif nvs_flash
)