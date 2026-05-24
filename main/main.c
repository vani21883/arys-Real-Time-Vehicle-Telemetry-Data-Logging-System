#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "imu.h"
#include "gps.h"
#include "can.h"
#include "sd_card.h"
#include "rpm.h"
#include "telemetry.h"
#include "sensor_fusion.h"
#include "fault_monitor.h"

void app_main(void)
{
    printf("Vehicle Telemetry System Initializing...\n");

    // TODO: Initialize drivers
    // TODO: Spawn FreeRTOS tasks
}