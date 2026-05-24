#include "sensor_fusion.h"
#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Complementary filter coefficient (0.98 = trust gyro 98%, accel 2%) */
#define ALPHA   0.98f
#define DT_S    0.01f   /* 10 ms sample period */

static fusion_output_t s_output = {0};

esp_err_t sensor_fusion_init(void)
{
    printf("[FUSION] init placeholder\n");
    return ESP_OK;
}

void sensor_fusion_update(const imu_data_t *imu, const gps_data_t *gps)
{
    /* TODO: complementary filter
     *   angle = ALPHA * (angle + gyro * DT) + (1-ALPHA) * accel_angle
     *   g_force = sqrt(ax^2 + ay^2 + az^2) / 9.81
     *   distance integration from GPS speed
     */
    (void)imu; (void)gps;
}

void sensor_fusion_get(fusion_output_t *out)
{
    if (out) *out = s_output;
}

void sensor_fusion_task(void *pvParameters)
{
    imu_data_t  imu_data;
    gps_data_t  gps_data;
    for (;;) {
        imu_read(&imu_data);
        gps_get_latest(&gps_data);
        sensor_fusion_update(&imu_data, &gps_data);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}