#include "rpm.h"

#include <stdio.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_timer.h"

#include "shared_data.h"
#include "freertos/semphr.h"

#define TAG "RPM"

/* PCNT configuration */
#define RPM_PCNT_HIGH_LIMIT      10000
#define RPM_PCNT_LOW_LIMIT      -1

/* Sampling interval */
#define RPM_SAMPLE_TIME_MS       100

/* Timeout detection */
#define RPM_TIMEOUT_MS           2000

static volatile float s_rpm = 0.0f;

static pcnt_unit_handle_t s_pcnt_unit = NULL;
static int64_t s_last_pulse_time_ms = 0;

/* ---------------------------------------------------------- */
/* Utility function */
/* ---------------------------------------------------------- */

static int64_t get_time_ms(void)
{
    return (esp_timer_get_time() / 1000);
}

/* ---------------------------------------------------------- */
/* RPM initialization */
/* ---------------------------------------------------------- */

esp_err_t rpm_init(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Initializing RPM module");

    /* ------------------------------------------------------ */
    /* Configure GPIO */
    /* ------------------------------------------------------ */

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RPM_HALL_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ret = gpio_config(&io_conf);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO configuration failed");
        return ret;
    }

    /* ------------------------------------------------------ */
    /* Create PCNT unit */
    /* ------------------------------------------------------ */

    pcnt_unit_config_t unit_config = {
        .high_limit = RPM_PCNT_HIGH_LIMIT,
        .low_limit = RPM_PCNT_LOW_LIMIT
    };

    ret = pcnt_new_unit(&unit_config, &s_pcnt_unit);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCNT unit creation failed");
        return ret;
    }

    /* ------------------------------------------------------ */
    /* Configure PCNT channel */
    /* ------------------------------------------------------ */

    pcnt_chan_config_t chan_config = {
        .edge_gpio_num = RPM_HALL_PIN,
        .level_gpio_num = -1
    };

    pcnt_channel_handle_t pcnt_chan = NULL;

    ret = pcnt_new_channel(s_pcnt_unit,
                           &chan_config,
                           &pcnt_chan);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCNT channel creation failed");
        return ret;
    }

    /* Count on falling edge */
    ret = pcnt_channel_set_edge_action(
        pcnt_chan,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE,
        PCNT_CHANNEL_EDGE_ACTION_HOLD
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCNT edge action configuration failed");
        return ret;
    }

    /* ------------------------------------------------------ */
    /* Glitch filter */
    /* Removes noise / bouncing */
    /* ------------------------------------------------------ */

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000
    };

    ret = pcnt_unit_set_glitch_filter(
        s_pcnt_unit,
        &filter_config
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCNT glitch filter configuration failed");
        return ret;
    }

    /* ------------------------------------------------------ */
    /* Enable and start PCNT */
    /* ------------------------------------------------------ */

    ret = pcnt_unit_enable(s_pcnt_unit);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCNT enable failed");
        return ret;
    }

    ret = pcnt_unit_clear_count(s_pcnt_unit);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCNT clear failed");
        return ret;
    }

    ret = pcnt_unit_start(s_pcnt_unit);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCNT start failed");
        return ret;
    }

    s_last_pulse_time_ms = get_time_ms();

    ESP_LOGI(TAG, "RPM module initialized successfully");

    return ESP_OK;
}

/* ---------------------------------------------------------- */
/* Return current RPM */
/* ---------------------------------------------------------- */

float rpm_get(void)
{
    return s_rpm;
}

/* ---------------------------------------------------------- */
/* Convert RPM to speed */
/* ---------------------------------------------------------- */

float rpm_get_speed_kmh(float wheel_circumference_m)
{
    return (s_rpm * wheel_circumference_m * 60.0f) / 1000.0f;
}

/* ---------------------------------------------------------- */
/* RPM Task */
/* ---------------------------------------------------------- */

void rpm_task(void *pvParameters)
{
    int pulse_count = 0;

    while (1) {

        vTaskDelay(pdMS_TO_TICKS(RPM_SAMPLE_TIME_MS));

        /* -------------------------------------- */
        /* Read PCNT */
        /* -------------------------------------- */

        pcnt_unit_get_count(
            s_pcnt_unit,
            &pulse_count
        );

        pcnt_unit_clear_count(s_pcnt_unit);

        pulse_count = abs(pulse_count);

        /* -------------------------------------- */
        /* RPM Calculation */
        /* -------------------------------------- */

        s_rpm =
            ((float)pulse_count / RPM_PULSES_PER_REV)
            * (60000.0f / RPM_SAMPLE_TIME_MS);

        /* -------------------------------------- */
        /* Timeout Detection */
        /* -------------------------------------- */

        if (pulse_count > 0) {
            s_last_pulse_time_ms = get_time_ms();
        }

        int64_t elapsed =
            get_time_ms() - s_last_pulse_time_ms;

        if (elapsed > RPM_TIMEOUT_MS) {

            s_rpm = 0.0f;

            ESP_LOGW(
                TAG,
                "No RPM pulses detected "
                "(%lld ms timeout)",
                elapsed
            );
        }

        /* -------------------------------------- */
        /* Update Shared Data */
        /* -------------------------------------- */

        xSemaphoreTake(
            g_telemetry_mutex,
            portMAX_DELAY
        );

        g_telemetry_data.rpm = s_rpm;

        g_telemetry_data.timestamp_ms =
            esp_timer_get_time() / 1000;

        g_telemetry_data.rpm_last_update_ms = esp_timer_get_time() / 1000;

        xSemaphoreGive(g_telemetry_mutex);

        /* -------------------------------------- */
        /* Debug Logging */
        /* -------------------------------------- */

        ESP_LOGI(
            TAG,
            "Pulse Count: %d | RPM: %.2f",
            pulse_count,
            s_rpm
        );
    }
}

/*void rpm_task(void *pvParameters)
{
    int pulse_count = 0;

    while (1) {

        vTaskDelay(pdMS_TO_TICKS(RPM_SAMPLE_TIME_MS));

        pcnt_unit_get_count(s_pcnt_unit, &pulse_count);

        pcnt_unit_clear_count(s_pcnt_unit);


        pulse_count = abs(pulse_count);

        s_rpm =
            ((float)pulse_count / RPM_PULSES_PER_REV)
            * (60000.0f / RPM_SAMPLE_TIME_MS);

        if (pulse_count > 0) {
            s_last_pulse_time_ms = get_time_ms();
        }

        int64_t elapsed =
            get_time_ms() - s_last_pulse_time_ms;

        if (elapsed > RPM_TIMEOUT_MS) {

            s_rpm = 0.0f;

            ESP_LOGW(TAG,
                     "No RPM pulses detected (%lld ms timeout)",
                     elapsed);

        
        }

        ESP_LOGI(TAG,
                 "Pulse Count: %d | RPM: %.2f",
                 pulse_count,
                 s_rpm);
    }
} */

/*#include "rpm.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "driver/gpio.h"        // still valid path in 6.0
#include "driver/pulse_cnt.h"   // keep this, it's correct
 
static volatile float s_rpm = 0.0f;
 
esp_err_t rpm_init(void)
{
    //TODO: configure PCNT unit on RPM_HALL_PIN, set filter, enable interrupt 
    printf("[RPM] init placeholder\n");
    return ESP_OK;
}
 
float rpm_get(void)
{
    return s_rpm;
}
 
float rpm_get_speed_kmh(float wheel_circumference_m)
{
    //speed = (RPM * circumference * 60) / 1000
    return (s_rpm * wheel_circumference_m * 60.0f) / 1000.0f;
}
 
void rpm_task(void *pvParameters)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
*/