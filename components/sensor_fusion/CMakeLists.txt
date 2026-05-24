idf_component_register(
    SRCS "sensor_fusion.c"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_common imu gps
)