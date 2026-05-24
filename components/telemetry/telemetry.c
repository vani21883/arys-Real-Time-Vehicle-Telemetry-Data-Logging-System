#include "telemetry.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"

esp_err_t telemetry_init(void)
{
    /* TODO: nvs_flash_init, esp_netif_init, wifi_init_softap or sta */
    printf("[TELEMETRY] init placeholder\n");
    return ESP_OK;
}

esp_err_t telemetry_send(const char *json_payload, uint32_t len)
{
    /* TODO: open UDP socket, sendto() broadcast/host */
    (void)json_payload; (void)len;
    return ESP_OK;
}

void telemetry_task(void *pvParameters)
{
    char payload[512];
    for (;;) {
        /* TODO: build JSON from shared sensor data, call telemetry_send() */
        (void)payload;
        vTaskDelay(pdMS_TO_TICKS(100)); /* 10 Hz telemetry */
    }
}