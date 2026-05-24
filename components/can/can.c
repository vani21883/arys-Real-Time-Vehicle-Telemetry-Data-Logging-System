#pragma GCC diagnostic ignored "-Wcpp"

#include "can.h"

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/twai.h"

#include "esp_log.h"
#include "esp_err.h"

#define TAG "CAN"

/* CAN Bus Bitrate */
#define CAN_BUS_SPEED_CONFIG     TWAI_TIMING_CONFIG_500KBITS()

/* Polling timeout */
#define CAN_RX_TIMEOUT_MS        100
#define CAN_ALERT_TIMEOUT_MS     1000

/* Internal driver state */
static bool can_driver_installed = false;

/* ---------------------------------------------------------- */
/* CAN Initialization */
/* ---------------------------------------------------------- */

esp_err_t can_init(void)
{
    ESP_LOGI(TAG, "Initializing TWAI CAN driver...");

    /*
     * SN65HVD230 Notes:
     * - RS pin tied to GND -> High-speed mode
     * - ESP32 TWAI handles CAN protocol
     * - SN65HVD230 acts as physical layer transceiver
     */

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        CAN_TX_PIN,
        CAN_RX_PIN,
        TWAI_MODE_NORMAL
    );

    twai_timing_config_t t_config = CAN_BUS_SPEED_CONFIG;

    /*
     * Accept all CAN frames
     */
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    /* Install TWAI driver */
    esp_err_t ret = twai_driver_install(
        &g_config,
        &t_config,
        &f_config
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install TWAI driver");
        return ret;
    }

    ESP_LOGI(TAG, "TWAI driver installed");

    /* Start TWAI driver */
    ret = twai_start();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start TWAI driver");
        twai_driver_uninstall();
        return ret;
    }

    ESP_LOGI(TAG, "TWAI driver started");

    /*
     * Enable alerts:
     * - RX data available
     * - Bus errors
     * - Error passive state
     * - RX queue full
     * - Bus recovery
     */

    uint32_t alerts_to_enable =
        TWAI_ALERT_RX_DATA |
        TWAI_ALERT_ERR_PASS |
        TWAI_ALERT_BUS_ERROR |
        TWAI_ALERT_RX_QUEUE_FULL |
        TWAI_ALERT_BUS_RECOVERED |
        TWAI_ALERT_ABOVE_ERR_WARN;

    ret = twai_reconfigure_alerts(alerts_to_enable, NULL);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure TWAI alerts");
        twai_stop();
        twai_driver_uninstall();
        return ret;
    }

    can_driver_installed = true;

    ESP_LOGI(TAG, "CAN initialization successful");

    return ESP_OK;
}

/* ---------------------------------------------------------- */
/* CAN Transmit */
/* ---------------------------------------------------------- */

esp_err_t can_send(const can_frame_t *frame)
{
    if (frame == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!can_driver_installed) {
        return ESP_ERR_INVALID_STATE;
    }

    twai_message_t message = {
        .identifier = frame->id,
        .data_length_code = frame->dlc,
        .extd = 0,
        .rtr = 0
    };

    memcpy(message.data, frame->data, frame->dlc);

    esp_err_t ret = twai_transmit(
        &message,
        pdMS_TO_TICKS(100)
    );

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "CAN frame transmitted | ID: 0x%lx", frame->id);
    } else {
        ESP_LOGE(TAG, "CAN transmit failed");
    }

    return ret;
}

/* ---------------------------------------------------------- */
/* CAN Receive */
/* ---------------------------------------------------------- */

esp_err_t can_receive(can_frame_t *frame, uint32_t timeout_ms)
{
    if (frame == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    twai_message_t message;

    esp_err_t ret = twai_receive(
        &message,
        pdMS_TO_TICKS(timeout_ms)
    );

    if (ret != ESP_OK) {
        return ret;
    }

    frame->id  = message.identifier;
    frame->dlc = message.data_length_code;

    memcpy(frame->data, message.data, frame->dlc);

    return ESP_OK;
}

/* ---------------------------------------------------------- */
/* CAN Task */
/* ---------------------------------------------------------- */

void can_task(void *pvParameters)
{
    can_frame_t frame;

    uint32_t alerts_triggered;

    twai_status_info_t status_info;

    ESP_LOGI(TAG, "CAN task started");

    while (1) {

        /* -------------------------------------- */
        /* Read CAN Alerts */
        /* -------------------------------------- */

        if (twai_read_alerts(
                &alerts_triggered,
                pdMS_TO_TICKS(CAN_ALERT_TIMEOUT_MS)
            ) == ESP_OK)
        {

            twai_get_status_info(&status_info);

            /* ---------------------------------- */
            /* RX Data Available */
            /* ---------------------------------- */

            if (alerts_triggered & TWAI_ALERT_RX_DATA) {

                while (can_receive(&frame, 0) == ESP_OK) {

                    ESP_LOGI(TAG,
                             "RX | ID: 0x%lx | DLC: %d",
                             frame.id,
                             frame.dlc);

                    printf("DATA: ");

                    for (int i = 0; i < frame.dlc; i++) {
                        printf("%02X ", frame.data[i]);
                    }

                    printf("\n");

                    /*
                     * Future:
                     * - Send to telemetry module
                     * - Log to SD card
                     * - Sensor processing
                     */
                }
            }

            /* ---------------------------------- */
            /* Bus Error */
            /* ---------------------------------- */

            if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {

                ESP_LOGW(TAG,
                         "CAN Bus Error | Count: %lu",
                         status_info.bus_error_count);
            }

            /* ---------------------------------- */
            /* Error Passive */
            /* ---------------------------------- */

            if (alerts_triggered & TWAI_ALERT_ERR_PASS) {

                ESP_LOGW(TAG,
                         "CAN entered ERROR PASSIVE state");
            }

            /* ---------------------------------- */
            /* RX Queue Full */
            /* ---------------------------------- */

            if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {

                ESP_LOGW(TAG,
                         "RX Queue Full");

                ESP_LOGW(TAG,
                         "Missed: %lu | Overrun: %lu",
                         status_info.rx_missed_count,
                         status_info.rx_overrun_count);
            }

            /* ---------------------------------- */
            /* Above Error Warning */
            /* ---------------------------------- */

            if (alerts_triggered & TWAI_ALERT_ABOVE_ERR_WARN) {

                ESP_LOGW(TAG,
                         "CAN above error warning limit");
            }

            /* ---------------------------------- */
            /* Bus Recovered */
            /* ---------------------------------- */

            if (alerts_triggered & TWAI_ALERT_BUS_RECOVERED) {

                ESP_LOGI(TAG,
                         "CAN bus recovered");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*
#include "can.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "driver/twai.h"
 
esp_err_t can_init(void)
{
    // TODO: configure TWAI general, timing, filter configs; twai_driver_install + twai_start
    printf("[CAN] init placeholder\n");
    return ESP_OK;
}
 
esp_err_t can_send(const can_frame_t *frame)
{
    // TODO: fill twai_message_t and call twai_transmit() 
    (void)frame;
    return ESP_OK;
}
 
esp_err_t can_receive(can_frame_t *frame, uint32_t timeout_ms)
{
    // TODO: twai_receive() and copy to frame
    (void)frame;
    (void)timeout_ms;
    return ESP_OK;
}
 
void can_task(void *pvParameters)
{
    can_frame_t frame;
    for (;;) {
        can_receive(&frame, 100);
        // TODO: dispatch frame to appropriate handler
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
*/