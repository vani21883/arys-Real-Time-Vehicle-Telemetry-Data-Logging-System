#ifndef FAULT_MONITOR_H
#define FAULT_MONITOR_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

typedef enum {
    FAULT_NONE          = 0,
    FAULT_IMU_TIMEOUT   = (1 << 0),
    FAULT_GPS_LOSS      = (1 << 1),
    FAULT_CAN_TIMEOUT   = (1 << 2),
    FAULT_SD_WRITE_FAIL = (1 << 3),
    FAULT_RPM_DROPOUT   = (1 << 4),
} fault_flag_t;

esp_err_t   fault_monitor_init(void);
void        fault_set(fault_flag_t flag);
void        fault_clear(fault_flag_t flag);
uint32_t    fault_get_all(void);
bool        fault_is_set(fault_flag_t flag);
void        fault_monitor_task(void *pvParameters);

#endif /* FAULT_MONITOR_H */