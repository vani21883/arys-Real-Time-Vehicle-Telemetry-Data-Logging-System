#include "sd_card.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
 
esp_err_t sd_card_init(void)
{
    /* TODO: sdmmc_host_t + sdspi_device_config_t, esp_vfs_fat_sdspi_mount() */
    printf("[SD] init placeholder\n");
    return ESP_OK;
}
 
esp_err_t sd_card_write(const char *filename, const char *data, uint32_t len)
{
    /* TODO: fopen(filename, "a"), fwrite, fclose */
    (void)filename; (void)data; (void)len;
    return ESP_OK;
}
 
esp_err_t sd_card_deinit(void)
{
    /* TODO: esp_vfs_fat_sdcard_unmount() */
    return ESP_OK;
}
 
void sd_card_task(void *pvParameters)
{
    for (;;) {
        /* TODO: receive log entries from queue, batch-write to SD */
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}