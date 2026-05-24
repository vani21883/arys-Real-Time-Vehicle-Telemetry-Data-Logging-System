#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>
#include "esp_err.h"

#define TELEMETRY_WIFI_SSID     "TelemetryAP"
#define TELEMETRY_WIFI_PASS     "telemetry123"
#define TELEMETRY_UDP_PORT      5005

esp_err_t   telemetry_init(void);
esp_err_t   telemetry_send(const char *json_payload, uint32_t len);
void        telemetry_task(void *pvParameters);

#endif /* TELEMETRY_H */