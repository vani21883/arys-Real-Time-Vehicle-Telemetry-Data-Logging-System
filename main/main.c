#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "imu.h"
#include "gps.h"
#include "can.h"
#include "sd_card.h"
#include "rpm.h"
#include "telemetry.h"
#include "sensor_fusion.h"
#include "fault_monitor.h"

#define TAG "MAIN"

void app_main(void)
{
    ESP_LOGI(TAG, "Vehicle Telemetry System Initializing...");

    esp_err_t ret = imu_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "IMU initialization failed!");
        return;
    }

    xTaskCreate(
        imu_task,
        "imu_task",
        4096,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG, "IMU task started");
}