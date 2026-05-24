#include "fault_monitor.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
    for (;;) {
        /* TODO:
         * - Check IMU last-seen timestamp → set FAULT_IMU_TIMEOUT
         * - Check GPS fix_valid        → set FAULT_GPS_LOSS
         * - Check CAN last-rx time     → set FAULT_CAN_TIMEOUT
         * - Check SD write results     → set FAULT_SD_WRITE_FAIL
         */
        if (s_fault_flags != FAULT_NONE) {
            printf("[FAULT] Active faults: 0x%02lX\n", (unsigned long)s_fault_flags);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}