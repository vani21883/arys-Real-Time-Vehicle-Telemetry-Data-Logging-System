#include "fault_monitor.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "shared_data.h"

static volatile uint32_t s_fault_flags = FAULT_NONE;

esp_err_t fault_monitor_init(void)
{
    s_fault_flags = FAULT_NONE;
    printf("[FAULT] init placeholder\n");
    return ESP_OK;
}

void fault_set(fault_flag_t flag)
{
    s_fault_flags |= (uint32_t)flag;
    printf("[FAULT] SET: 0x%02lX\n", (unsigned long)flag);
}

void fault_clear(fault_flag_t flag)
{
    s_fault_flags &= ~(uint32_t)flag;
}

uint32_t fault_get_all(void)
{
    return s_fault_flags;
}

bool fault_is_set(fault_flag_t flag)
{
    return (s_fault_flags & (uint32_t)flag) != 0;
}

void fault_monitor_task(void *pvParameters)
{
    const uint64_t timeout_ms = 2000;

    while (1) {

        uint64_t now_ms =
            esp_timer_get_time() / 1000;

        /* -------------------------------------- */
        /* IMU Timeout */
        /* -------------------------------------- */

        if ((now_ms -
             g_telemetry_data.imu_last_update_ms)
            > timeout_ms)
        {
            fault_set(FAULT_IMU_TIMEOUT);
        }
        else {
            fault_clear(FAULT_IMU_TIMEOUT);
        }

        /* -------------------------------------- */
        /* GPS Loss */
        /* -------------------------------------- */

        if (!g_telemetry_data.gps_fix_valid) {

            fault_set(FAULT_GPS_LOSS);

        } else {

            fault_clear(FAULT_GPS_LOSS);
        }

        /* -------------------------------------- */
        /* CAN Timeout */
        /* -------------------------------------- */

        if ((now_ms -
             g_telemetry_data.can_last_update_ms)
            > timeout_ms)
        {
            fault_set(FAULT_CAN_TIMEOUT);
        }
        else {
            fault_clear(FAULT_CAN_TIMEOUT);
        }

        /* -------------------------------------- */
        /* RPM Dropout */
        /* -------------------------------------- */

        if (g_telemetry_data.rpm <= 0.0f) {

            fault_set(FAULT_RPM_DROPOUT);

        } else {

            fault_clear(FAULT_RPM_DROPOUT);
        }

        /* -------------------------------------- */
        /* Print Active Faults */
        /* -------------------------------------- */

        if (s_fault_flags != FAULT_NONE) {

            ESP_LOGW(
                "FAULT",
                "Active Faults: 0x%02lX",
                (unsigned long)s_fault_flags
            );
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}