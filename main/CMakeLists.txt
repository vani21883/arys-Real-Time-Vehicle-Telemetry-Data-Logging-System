idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        imu
        gps
        can
        sd_card
        rpm
        telemetry
        sensor_fusion
        fault_monitor
)