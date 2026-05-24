#ifndef SENSOR_FUSION_H
#define SENSOR_FUSION_H

#include <stdint.h>
#include "esp_err.h"
#include "imu.h"
#include "gps.h"

typedef struct {
    float roll_deg;
    float pitch_deg;
    float yaw_deg;
    float accel_ms2;        /* resultant acceleration */
    float g_force;
    float distance_m;
} fusion_output_t;

esp_err_t   sensor_fusion_init(void);
void        sensor_fusion_update(const imu_data_t *imu, const gps_data_t *gps);
void        sensor_fusion_get(fusion_output_t *out);
void        sensor_fusion_task(void *pvParameters);

#endif /* SENSOR_FUSION_H */