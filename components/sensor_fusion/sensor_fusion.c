#include "sensor_fusion.h"
#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/* Complementary filter coefficient (0.98 = trust gyro 98%, accel 2%) */
#define ALPHA   0.98f
#define DT_S    0.01f   /* 10 ms sample period */

static fusion_output_t s_output = {0};

esp_err_t sensor_fusion_init(void)
{
    printf("[FUSION] init placeholder\n");
    return ESP_OK;
}

void sensor_fusion_update(
    const imu_data_t *imu,
    const gps_data_t *gps
)
{
    if (imu == NULL || gps == NULL) {
        return;
    }

    /* -------------------------------------------------- */
    /* Accelerometer Angles */
    /* -------------------------------------------------- */

    float accel_roll =
        atan2f(
            imu->accel_y,
            imu->accel_z
        ) * (180.0f / M_PI);

    float accel_pitch =
        atan2f(
            -imu->accel_x,
            sqrtf(
                (imu->accel_y * imu->accel_y) +
                (imu->accel_z * imu->accel_z)
            )
        ) * (180.0f / M_PI);

    /* -------------------------------------------------- */
    /* Complementary Filter */
    /* -------------------------------------------------- */

    s_output.roll_deg =
        ALPHA *
        (s_output.roll_deg + (imu->gyro_x * DT_S))
        +
        ((1.0f - ALPHA) * accel_roll);

    s_output.pitch_deg =
        ALPHA *
        (s_output.pitch_deg + (imu->gyro_y * DT_S))
        +
        ((1.0f - ALPHA) * accel_pitch);

    /* -------------------------------------------------- */
    /* Heading/Yaw from GPS */
    /* -------------------------------------------------- */

    s_output.yaw_deg =
        gps->heading_deg;

    /* -------------------------------------------------- */
    /* Resultant Acceleration */
    /* -------------------------------------------------- */

    s_output.accel_ms2 =
        sqrtf(
            (imu->accel_x * imu->accel_x) +
            (imu->accel_y * imu->accel_y) +
            (imu->accel_z * imu->accel_z)
        );

    /* -------------------------------------------------- */
    /* G-Force */
    /* -------------------------------------------------- */

    s_output.g_force =
        s_output.accel_ms2 / 9.80665f;

    /* -------------------------------------------------- */
    /* Distance Estimation */
    /* -------------------------------------------------- */

    /*
     * speed_kmh -> m/s
     */

    float speed_ms =
        gps->speed_kmh / 3.6f;

    s_output.distance_m +=
        speed_ms * DT_S;
}

void sensor_fusion_get(fusion_output_t *out)
{
    if (out) *out = s_output;
}

void sensor_fusion_task(void *pvParameters)
{
    imu_data_t imu_data;
    gps_data_t gps_data;

    while (1) {

        imu_read(&imu_data);

        gps_get_latest(&gps_data);

        sensor_fusion_update(
            &imu_data,
            &gps_data
        );

        ESP_LOGI(
            "FUSION",
            "ROLL: %.2f | "
            "PITCH: %.2f | "
            "YAW: %.2f | "
            "G-FORCE: %.2f | "
            "DIST: %.2f m",

            s_output.roll_deg,
            s_output.pitch_deg,
            s_output.yaw_deg,
            s_output.g_force,
            s_output.distance_m
        );

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}