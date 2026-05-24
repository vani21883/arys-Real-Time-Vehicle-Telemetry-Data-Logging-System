#include "sd_card.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "shared_data.h"

#if !SD_SIMULATION_MODE

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_master.h"

#endif

#define TAG "SD_CARD"

/* ---------------------------------------------------------- */
/* SD Card Handle */
/* ---------------------------------------------------------- */

#if !SD_SIMULATION_MODE
static sdmmc_card_t *s_card;
#endif

/* ---------------------------------------------------------- */
/* SD Card Initialization */
/* ---------------------------------------------------------- */

esp_err_t sd_card_init(void)
{
#if SD_SIMULATION_MODE

    ESP_LOGW(TAG,
             "SD card running in simulation mode");

    return ESP_OK;

#else

    esp_err_t ret;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_MOSI_PIN,
        .miso_io_num = SD_MISO_PIN,
        .sclk_io_num = SD_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(
        SPI2_HOST,
        &bus_cfg,
        SPI_DMA_CH_AUTO
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG,
                 "SPI bus init failed");
        return ret;
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    sdspi_device_config_t slot_config =
        SDSPI_DEVICE_CONFIG_DEFAULT();

    slot_config.gpio_cs = SD_CS_PIN;
    slot_config.host_id = SPI2_HOST;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    ret = esp_vfs_fat_sdspi_mount(
        SD_MOUNT_POINT,
        &host,
        &slot_config,
        &mount_config,
        &s_card
    );

    if (ret != ESP_OK) {

        ESP_LOGE(TAG,
                 "SD mount failed");

        return ret;
    }

    ESP_LOGI(TAG,
             "SD card mounted");

    return ESP_OK;

#endif
}

/* ---------------------------------------------------------- */
/* SD Card Write */
/* ---------------------------------------------------------- */

esp_err_t sd_card_write(
    const char *filename,
    const char *data,
    uint32_t len
)
{
#if SD_SIMULATION_MODE

    ESP_LOGI(TAG,
             "SIM CSV: %s",
             data);

    return ESP_OK;

#else

    FILE *file;

    file = fopen(filename, "a");

    if (file == NULL) {

        ESP_LOGE(TAG,
                 "Failed to open file");

        return ESP_FAIL;
    }

    fwrite(data, 1, len, file);

    fclose(file);

    return ESP_OK;

#endif
}

/* ---------------------------------------------------------- */
/* SD Card Deinit */
/* ---------------------------------------------------------- */

esp_err_t sd_card_deinit(void)
{
#if SD_SIMULATION_MODE

    ESP_LOGI(TAG,
             "SD simulation deinit");

    return ESP_OK;

#else

    esp_vfs_fat_sdcard_unmount(
        SD_MOUNT_POINT,
        s_card
    );

    spi_bus_free(SPI2_HOST);

    ESP_LOGI(TAG,
             "SD card unmounted");

    return ESP_OK;

#endif
}

/* ---------------------------------------------------------- */
/* SD Logger Task */
/* ---------------------------------------------------------- */

void sd_card_task(void *pvParameters)
{
    char csv_line[256];

    telemetry_data_t snapshot;

    ESP_LOGI(TAG,
             "SD logger task started");

    /* CSV Header */

    sd_card_write(
        SD_LOG_FILE,
        "timestamp_ms,rpm,accel_x,accel_y,accel_z,"
        "gyro_x,gyro_y,gyro_z,"
        "latitude,longitude,speed_kmh\n",
        strlen(
            "timestamp_ms,rpm,accel_x,accel_y,accel_z,"
            "gyro_x,gyro_y,gyro_z,"
            "latitude,longitude,speed_kmh\n"
        )
    );

    while (1) {

        /* ---------------------------------------------- */
        /* Copy shared telemetry safely */
        /* ---------------------------------------------- */

        xSemaphoreTake(
            g_telemetry_mutex,
            portMAX_DELAY
        );

        snapshot = g_telemetry_data;

        xSemaphoreGive(
            g_telemetry_mutex
        );

        /* ---------------------------------------------- */
        /* Generate CSV line */
        /* ---------------------------------------------- */

        snprintf(
            csv_line,
            sizeof(csv_line),

            "%" PRIu64 ","
            "%.2f,"
            "%.2f,%.2f,%.2f,"
            "%.2f,%.2f,%.2f,"
            "%.6lf,%.6lf,"
            "%.2f\n",

            snapshot.timestamp_ms,

            snapshot.rpm,

            snapshot.accel_x,
            snapshot.accel_y,
            snapshot.accel_z,

            snapshot.gyro_x,
            snapshot.gyro_y,
            snapshot.gyro_z,

            snapshot.latitude,
            snapshot.longitude,

            snapshot.speed_kmh
        );

        /* ---------------------------------------------- */
        /* Write CSV */
        /* ---------------------------------------------- */

        sd_card_write(
            SD_LOG_FILE,
            csv_line,
            strlen(csv_line)
        );

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/*
#include "sd_card.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
 
esp_err_t sd_card_init(void)
{
    // TODO: sdmmc_host_t + sdspi_device_config_t, esp_vfs_fat_sdspi_mount()
    printf("[SD] init placeholder\n");
    return ESP_OK;
}
 
esp_err_t sd_card_write(const char *filename, const char *data, uint32_t len)
{
    // TODO: fopen(filename, "a"), fwrite, fclose
    (void)filename; (void)data; (void)len;
    return ESP_OK;
}
 
esp_err_t sd_card_deinit(void)
{
    // TODO: esp_vfs_fat_sdcard_unmount()
    return ESP_OK;
}
 
void sd_card_task(void *pvParameters)
{
    for (;;) {
        // TODO: receive log entries from queue, batch-write to SD 
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

*/