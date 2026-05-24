idf_component_register(
    SRCS "fault_monitor.c"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_common esp_event esp_timer sd_card
)