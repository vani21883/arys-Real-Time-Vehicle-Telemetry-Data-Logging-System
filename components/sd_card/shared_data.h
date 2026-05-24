#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/* ---------------------------------------------------------- */
/* Shared Telemetry Structure */
/* ---------------------------------------------------------- */

typedef struct
{
    /* IMU */

    float accel_x;
    float accel_y;
    float accel_z;

    float gyro_x;
    float gyro_y;
    float gyro_z;

    float imu_temp_c;

    /* GPS */

    double latitude;
    double longitude;

    float altitude_m;
    float speed_kmh;
    float heading_deg;

    uint8_t satellites;
    bool gps_fix_valid;

    /* RPM */

    float rpm;

    /* System */

    uint64_t timestamp_ms;

    /* Last update timestamps */

    uint64_t imu_last_update_ms;
    uint64_t gps_last_update_ms;
    uint64_t can_last_update_ms;
    uint64_t rpm_last_update_ms;

} telemetry_data_t;

/* ---------------------------------------------------------- */
/* Global Shared Data */
/* ---------------------------------------------------------- */

extern telemetry_data_t g_telemetry_data;

/* ---------------------------------------------------------- */
/* Mutex Protection */
/* ---------------------------------------------------------- */

extern SemaphoreHandle_t g_telemetry_mutex;

#endif /* SHARED_DATA_H */