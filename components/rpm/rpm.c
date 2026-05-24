#include "rpm.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "driver/gpio.h"        // still valid path in 6.0
#include "driver/pulse_cnt.h"   // keep this, it's correct
 
static volatile float s_rpm = 0.0f;
 
esp_err_t rpm_init(void)
{
    /* TODO: configure PCNT unit on RPM_HALL_PIN, set filter, enable interrupt */
    printf("[RPM] init placeholder\n");
    return ESP_OK;
}
 
float rpm_get(void)
{
    return s_rpm;
}
 
float rpm_get_speed_kmh(float wheel_circumference_m)
{
    /* speed = (RPM * circumference * 60) / 1000 */
    return (s_rpm * wheel_circumference_m * 60.0f) / 1000.0f;
}
 
void rpm_task(void *pvParameters)
{
    for (;;) {
        /* TODO: read pulse count over 100ms window, calculate RPM, store in s_rpm */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}