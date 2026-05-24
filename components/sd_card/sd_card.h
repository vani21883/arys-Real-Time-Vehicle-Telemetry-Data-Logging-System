#ifndef SD_CARD_H
#define SD_CARD_H
 
#include <stdint.h>

#include "esp_err.h"
 
/* ---------------------------------------------------------- */
/* SPI Pins */
/* ---------------------------------------------------------- */

#define SD_MOSI_PIN       23
#define SD_MISO_PIN       19
#define SD_CLK_PIN        18
#define SD_CS_PIN         5

/* ---------------------------------------------------------- */
/* Filesystem */
/* ---------------------------------------------------------- */

#define SD_MOUNT_POINT    "/sdcard"
#define SD_LOG_FILE       "/sdcard/telemetry.csv"

/* ---------------------------------------------------------- */
/* Simulation Mode */
/* ---------------------------------------------------------- */

#define SD_SIMULATION_MODE    1

/* ---------------------------------------------------------- */
/* APIs */
/* ---------------------------------------------------------- */

esp_err_t sd_card_init(void);

esp_err_t sd_card_write(
    const char *filename,
    const char *data,
    uint32_t len
);

esp_err_t sd_card_deinit(void);

void sd_card_task(void *pvParameters);
 
#endif /* SD_CARD_H */

/*
#ifndef SD_CARD_H
#define SD_CARD_H
 
#include <stdint.h>
#include "esp_err.h"
 
#define SD_MOSI_PIN     23
#define SD_MISO_PIN     19
#define SD_CLK_PIN      18
#define SD_CS_PIN       5
#define SD_MOUNT_POINT  "/sdcard"
 
esp_err_t   sd_card_init(void);
esp_err_t   sd_card_write(const char *filename, const char *data, uint32_t len);
esp_err_t   sd_card_deinit(void);
void        sd_card_task(void *pvParameters);
 
#endif */