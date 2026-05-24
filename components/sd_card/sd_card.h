#ifndef SD_CARD_H
#define SD_CARD_H
 
#include <stdint.h>
#include "esp_err.h"
 
/* SPI pins for SD module */
#define SD_MOSI_PIN     23
#define SD_MISO_PIN     19
#define SD_CLK_PIN      18
#define SD_CS_PIN       5
#define SD_MOUNT_POINT  "/sdcard"
 
esp_err_t   sd_card_init(void);
esp_err_t   sd_card_write(const char *filename, const char *data, uint32_t len);
esp_err_t   sd_card_deinit(void);
void        sd_card_task(void *pvParameters);
 
#endif /* SD_CARD_H */