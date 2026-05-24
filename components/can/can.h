#ifndef CAN_H
#define CAN_H
 
#include <stdint.h>
#include "esp_err.h"
 
/* SN65HVD230 transceiver pins */
#define CAN_TX_PIN  5
#define CAN_RX_PIN  4
 
typedef struct {
    uint32_t id;
    uint8_t  dlc;
    uint8_t  data[8];
} can_frame_t;
 
esp_err_t   can_init(void);
esp_err_t   can_send(const can_frame_t *frame);
esp_err_t   can_receive(can_frame_t *frame, uint32_t timeout_ms);
void        can_task(void *pvParameters);
 
#endif /* CAN_H */