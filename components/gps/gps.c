#include "gps.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "esp_log.h"

#include "shared_data.h"
#include "freertos/semphr.h"
#include "esp_timer.h"

#define TAG "GPS"

#define GPS_UART_PORT      UART_NUM_1
#define GPS_TX_PIN         17
#define GPS_RX_PIN         16

#define GPS_BAUD_RATE      9600
#define GPS_BUF_SIZE       1024

static gps_data_t s_latest = {0};

/* ---------------------------------------------------------- */
/* Convert NMEA DDMM.MMMM -> Decimal Degrees */
/* ---------------------------------------------------------- */

static double nmea_to_decimal(const char *coord, char direction)
{
    if (coord == NULL || strlen(coord) < 3)
        return 0.0;

    double raw = atof(coord);

    int degrees = (int)(raw / 100);
    double minutes = raw - (degrees * 100);

    double decimal = degrees + (minutes / 60.0);

    if (direction == 'S' || direction == 'W')
        decimal *= -1.0;

    return decimal;
}

/* ---------------------------------------------------------- */
/* Parse GPRMC */
/* ---------------------------------------------------------- */

static void parse_gprmc(char *sentence)
{
    char *token;
    int field = 0;

    char latitude[20] = {0};
    char longitude[20] = {0};

    char lat_dir = 'N';
    char lon_dir = 'E';

    token = strtok(sentence, ",");

    while (token != NULL) {

        switch (field) {

            case 2:
                s_latest.fix_valid = (token[0] == 'A');
                break;

            case 3:
                strncpy(latitude, token, sizeof(latitude) - 1);
                break;

            case 4:
                lat_dir = token[0];
                break;

            case 5:
                strncpy(longitude, token, sizeof(longitude) - 1);
                break;

            case 6:
                lon_dir = token[0];
                break;

            case 7:
                s_latest.speed_kmh = atof(token) * 1.852f;
                break;

            case 8:
                s_latest.heading_deg = atof(token);
                break;

            default:
                break;
        }

        token = strtok(NULL, ",");
        field++;
    }

    s_latest.latitude = nmea_to_decimal(latitude, lat_dir);
    s_latest.longitude = nmea_to_decimal(longitude, lon_dir);
}

/* ---------------------------------------------------------- */
/* Parse GPGGA */
/* ---------------------------------------------------------- */

static void parse_gpgga(char *sentence)
{
    char *token;
    int field = 0;

    token = strtok(sentence, ",");

    while (token != NULL) {

        switch (field) {

            case 7:
                s_latest.satellites = atoi(token);
                break;

            case 9:
                s_latest.altitude_m = atof(token);
                break;

            default:
                break;
        }

        token = strtok(NULL, ",");
        field++;
    }
}

/* ---------------------------------------------------------- */
/* Parse NMEA Sentences */
/* ---------------------------------------------------------- */

static void parse_nmea(char *sentence)
{
    if (strstr(sentence, "$GPRMC") != NULL) {

        parse_gprmc(sentence);

    } else if (strstr(sentence, "$GPGGA") != NULL) {

        parse_gpgga(sentence);
    }
}

/* ---------------------------------------------------------- */
/* GPS Init */
/* ---------------------------------------------------------- */

esp_err_t gps_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = GPS_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    ESP_ERROR_CHECK(
        uart_driver_install(
            GPS_UART_PORT,
            GPS_BUF_SIZE * 2,
            0,
            0,
            NULL,
            0
        )
    );

    ESP_ERROR_CHECK(
        uart_param_config(
            GPS_UART_PORT,
            &uart_config
        )
    );

    ESP_ERROR_CHECK(
        uart_set_pin(
            GPS_UART_PORT,
            GPS_TX_PIN,
            GPS_RX_PIN,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE
        )
    );

    ESP_LOGI(TAG, "GPS UART initialized");

    return ESP_OK;
}

/* ---------------------------------------------------------- */
/* Get Latest GPS Data */
/* ---------------------------------------------------------- */

esp_err_t gps_get_latest(gps_data_t *out)
{
    if (out == NULL)
        return ESP_ERR_INVALID_ARG;

    *out = s_latest;

    return ESP_OK;
}

/* ---------------------------------------------------------- */
/* GPS Task */
/* ---------------------------------------------------------- */
void gps_task(void *pvParameters)
{
    uint8_t *data = (uint8_t *)malloc(GPS_BUF_SIZE);

    if (data == NULL) {

        ESP_LOGE(TAG,
                 "Failed to allocate GPS buffer");

        vTaskDelete(NULL);
        return;
    }

    char line[128];
    int line_pos = 0;

    while (1) {

        int len = uart_read_bytes(
            GPS_UART_PORT,
            data,
            GPS_BUF_SIZE - 1,
            pdMS_TO_TICKS(100)
        );

        if (len > 0) {

            for (int i = 0; i < len; i++) {

                char c = (char)data[i];

                if (c == '\n') {

                    line[line_pos] = '\0';

                    parse_nmea(line);

                    /* ---------------------------------- */
                    /* Update Shared Data */
                    /* ---------------------------------- */

                    xSemaphoreTake(
                        g_telemetry_mutex,
                        portMAX_DELAY
                    );

                    g_telemetry_data.latitude =
                        s_latest.latitude;

                    g_telemetry_data.longitude =
                        s_latest.longitude;

                    g_telemetry_data.altitude_m =
                        s_latest.altitude_m;

                    g_telemetry_data.speed_kmh =
                        s_latest.speed_kmh;

                    g_telemetry_data.heading_deg =
                        s_latest.heading_deg;

                    g_telemetry_data.satellites =
                        s_latest.satellites;

                    g_telemetry_data.gps_fix_valid =
                        s_latest.fix_valid;

                    g_telemetry_data.timestamp_ms =
                        esp_timer_get_time() / 1000;

                    g_telemetry_data.gps_last_update_ms = esp_timer_get_time() / 1000;

                    xSemaphoreGive(g_telemetry_mutex);

                    ESP_LOGI(
                        TAG,
                        "Lat: %.6f Lon: %.6f "
                        "Speed: %.2f km/h "
                        "Alt: %.2f m "
                        "Sats: %d",

                        s_latest.latitude,
                        s_latest.longitude,
                        s_latest.speed_kmh,
                        s_latest.altitude_m,
                        s_latest.satellites
                    );

                    line_pos = 0;

                } else {

                    if (line_pos < sizeof(line) - 1) {
                        line[line_pos++] = c;
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/*void gps_task(void *pvParameters)
{
    uint8_t *data = (uint8_t *)malloc(GPS_BUF_SIZE);

    if (data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate GPS buffer");
        vTaskDelete(NULL);
        return;
    }

    char line[128];
    int line_pos = 0;

    while (1) {

        int len = uart_read_bytes(
            GPS_UART_PORT,
            data,
            GPS_BUF_SIZE - 1,
            pdMS_TO_TICKS(100)
        );

        if (len > 0) {

            for (int i = 0; i < len; i++) {

                char c = (char)data[i];

                if (c == '\n') {

                    line[line_pos] = '\0';

                    parse_nmea(line);

                    ESP_LOGI(TAG,
                             "Lat: %.6f Lon: %.6f Speed: %.2f km/h Alt: %.2f m Sats: %d",
                             s_latest.latitude,
                             s_latest.longitude,
                             s_latest.speed_kmh,
                             s_latest.altitude_m,
                             s_latest.satellites);

                    line_pos = 0;

                } else {

                    if (line_pos < sizeof(line) - 1) {
                        line[line_pos++] = c;
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
    */