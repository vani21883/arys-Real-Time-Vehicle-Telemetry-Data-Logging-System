#include "gps.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

#define GPS_UART_PORT   UART_NUM_1
#define GPS_TX_PIN      17
#define GPS_RX_PIN      16
#define GPS_BAUD        9600
#define GPS_BUF_SIZE    512

static gps_data_t s_latest = {0};

esp_err_t gps_init(void)
{
    /* TODO: configure UART1 at 9600 8N1 for NEO-6M */
    printf("[GPS] init placeholder\n");
    return ESP_OK;
}

esp_err_t gps_get_latest(gps_data_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;
    *out = s_latest;
    return ESP_OK;
}

static void parse_nmea(const char *sentence)
{
    /* TODO: parse $GPRMC and $GPGGA sentences */
    (void)sentence;
}

void gps_task(void *pvParameters)
{
    uint8_t buf[GPS_BUF_SIZE];
    for (;;) {
        /* TODO: uart_read_bytes, feed lines to parse_nmea() */
        (void)buf;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}