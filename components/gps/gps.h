#ifndef GPS_H
#define GPS_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

typedef struct {
    double   latitude;
    double   longitude;
    float    altitude_m;
    float    speed_kmh;
    float    heading_deg;
    uint8_t  satellites;
    bool     fix_valid;
} gps_data_t;

esp_err_t   gps_init(void);
esp_err_t   gps_get_latest(gps_data_t *out);
void        gps_task(void *pvParameters);

#endif /* GPS_H */