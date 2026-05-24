idf_component_register(
    SRCS "can.c"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_common driver
)