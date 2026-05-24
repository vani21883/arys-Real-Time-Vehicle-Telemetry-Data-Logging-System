#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

/* Drivers */

#include "imu.h"
#include "gps.h"
#include "can.h"
#include "rpm.h"
#include "sd_card.h"
#include "shared_data.h"
#include "sensor_fusion.h"
#include "fault_monitor.h"

/* System Modules */

#include "telemetry.h"

#define TAG "MAIN"

void app_main(void)
{
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Vehicle Telemetry System Booting");
    ESP_LOGI(TAG, "=================================");

    esp_err_t ret;

    /* ------------------------------------------------------ */
    /* IMU */
    /* ------------------------------------------------------ */

    ESP_LOGI(TAG, "Initializing IMU");

    ret = imu_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "IMU init failed");
        return;
    }

    /* ------------------------------------------------------ */
    /* RPM */
    /* ------------------------------------------------------ */

    ESP_LOGI(TAG, "Initializing RPM");

    ret = rpm_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RPM init failed");
        return;
    }

    /* ------------------------------------------------------ */
    /* GPS */
    /* ------------------------------------------------------ */

    ESP_LOGI(TAG, "Initializing GPS");

    ret = gps_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPS init failed");
        return;
    }

    /* ------------------------------------------------------ */
    /* CAN */
    /* ------------------------------------------------------ */

    ESP_LOGI(TAG, "Initializing CAN");

    ret = can_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CAN init failed");
        return;
    }

    /* ------------------------------------------------------ */
    /* TELEMETRY */
    /* ------------------------------------------------------ */

    ESP_LOGI(TAG, "Initializing telemetry");

    ret = telemetry_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Telemetry init failed");
        return;
    }

    ESP_LOGI(TAG, "Initializing Sensor Fusion");

ret = sensor_fusion_init();

if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Sensor fusion init failed");
    return;
}

ESP_LOGI(TAG, "Initializing Fault Monitor");

ret = fault_monitor_init();

if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Fault monitor init failed");
    return;
}

    /* ------------------------------------------------------ */
    /* SD CARD */
    /* ------------------------------------------------------ */

    ESP_LOGI(TAG, "Initializing SD card");

    ret = sd_card_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SD card init failed");
        return;
    }

    g_telemetry_mutex = xSemaphoreCreateMutex();

if (g_telemetry_mutex == NULL) {

    ESP_LOGE(TAG,
             "Failed to create telemetry mutex");

    return;
}


    /* ------------------------------------------------------ */
    /* CREATE TASKS */
    /* ------------------------------------------------------ */

    xTaskCreate(
        imu_task,
        "imu_task",
        4096,
        NULL,
        5,
        NULL
    );

    xTaskCreate(
        rpm_task,
        "rpm_task",
        4096,
        NULL,
        4,
        NULL
    );

    xTaskCreate(
        gps_task,
        "gps_task",
        4096,
        NULL,
        4,
        NULL
    );

    xTaskCreate(
        can_task,
        "can_task",
        4096,
        NULL,
        5,
        NULL
    );

    xTaskCreate(
        telemetry_task,
        "telemetry_task",
        4096,
        NULL,
        3,
        NULL
    );

    xTaskCreate(
        sd_card_task,
        "sd_card_task",
        4096,
        NULL,
        2,
        NULL
    );

    xTaskCreate(
    sensor_fusion_task,
    "sensor_fusion_task",
    4096,
    NULL,
    3,
    NULL
);

xTaskCreate(
    fault_monitor_task,
    "fault_monitor_task",
    4096,
    NULL,
    3,
    NULL
);

    ESP_LOGI(TAG,
             "System initialization complete");
}