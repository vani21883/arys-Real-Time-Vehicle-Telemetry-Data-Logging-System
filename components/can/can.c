#include "can.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "driver/twai.h"
 
esp_err_t can_init(void)
{
    /* TODO: configure TWAI general, timing, filter configs; twai_driver_install + twai_start */
    printf("[CAN] init placeholder\n");
    return ESP_OK;
}
 
esp_err_t can_send(const can_frame_t *frame)
{
    /* TODO: fill twai_message_t and call twai_transmit() */
    (void)frame;
    return ESP_OK;
}
 
esp_err_t can_receive(can_frame_t *frame, uint32_t timeout_ms)
{
    /* TODO: twai_receive() and copy to frame */
    (void)frame;
    (void)timeout_ms;
    return ESP_OK;
}
 
void can_task(void *pvParameters)
{
    can_frame_t frame;
    for (;;) {
        can_receive(&frame, 100);
        /* TODO: dispatch frame to appropriate handler */
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
 