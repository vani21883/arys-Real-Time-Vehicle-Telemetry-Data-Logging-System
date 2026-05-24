#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

/* Drivers */
#include "imu.h"
#include "gps.h"
#include "can.h"
#include "sd_card.h"
#include "rpm.h"

/* System Modules */
#include "telemetry.h"
#include "sensor_fusion.h"
#include "fault_monitor.h"

#define TAG "MAIN"

void app_main(void)
{
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Vehicle Telemetry System Booting");
    ESP_LOGI(TAG, "=================================");

    esp_err_t ret;

    /* ------------------------------------------------------ */
    /* IMU Initialization */
    /* ------------------------------------------------------ */

    ESP_LOGI(TAG, "Initializing IMU...");

    ret = imu_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "IMU initialization failed!");
        return;
    }

    ESP_LOGI(TAG, "IMU initialized successfully");

    /* ------------------------------------------------------ */
    /* RPM Initialization */
    /* ------------------------------------------------------ */

    ESP_LOGI(TAG, "Initializing RPM module...");

    ret = rpm_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RPM initialization failed!");
        return;
    }

    ESP_LOGI(TAG, "RPM module initialized successfully");

    /* ------------------------------------------------------ */
    /* GPS Initialization */
    /* ------------------------------------------------------ */

    ESP_LOGI(TAG, "Initializing GPS module...");

    ret = gps_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPS initialization failed!");
        return;
    }

    ESP_LOGI(TAG, "GPS initialized successfully");

    /* ------------------------------------------------------ */
    /* CAN Initialization */
    /* ------------------------------------------------------ */

    ESP_LOGI(TAG, "Initializing CAN interface...");

    ret = can_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CAN initialization failed!");
        return;
    }

    ESP_LOGI(TAG, "CAN initialized successfully");

    /* ------------------------------------------------------ */
    /* Create IMU Task */
    /* ------------------------------------------------------ */

    xTaskCreate(
        imu_task,
        "imu_task",
        4096,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG, "IMU task started");

    /* ------------------------------------------------------ */
    /* Create RPM Task */
    /* ------------------------------------------------------ */

    xTaskCreate(
        rpm_task,
        "rpm_task",
        4096,
        NULL,
        4,
        NULL
    );

    ESP_LOGI(TAG, "RPM task started");

    /* ------------------------------------------------------ */
    /* Create GPS Task */
    /* ------------------------------------------------------ */

    xTaskCreate(
        gps_task,
        "gps_task",
        4096,
        NULL,
        4,
        NULL
    );

    ESP_LOGI(TAG, "GPS task started");

    /* ------------------------------------------------------ */
    /* Create CAN Task */
    /* ------------------------------------------------------ */

    xTaskCreate(
        can_task,
        "can_task",
        4096,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG, "CAN task started");

    /* ------------------------------------------------------ */
    /* Future Modules */
    /* ------------------------------------------------------ */

    /*
    sd_card_init();
    telemetry_init();
    sensor_fusion_init();
    fault_monitor_init();

    xTaskCreate(sd_logger_task, ...);
    xTaskCreate(telemetry_task, ...);
    xTaskCreate(fault_monitor_task, ...);
    */

    ESP_LOGI(TAG, "System initialization complete");
}