idf_component_register(
    SRCS "imu.c"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_common esp_driver_i2c
)