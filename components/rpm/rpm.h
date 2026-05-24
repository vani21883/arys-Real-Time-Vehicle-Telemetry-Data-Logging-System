#ifndef RPM_H
#define RPM_H
 
#include <stdint.h>
#include "esp_err.h"
 
#define RPM_HALL_PIN        34      /* GPIO input, hall sensor signal */
#define RPM_PULSES_PER_REV  1       /* adjust for magnet count */
 
esp_err_t   rpm_init(void);
float       rpm_get(void);          /* returns current RPM */
float       rpm_get_speed_kmh(float wheel_circumference_m);
void        rpm_task(void *pvParameters);
 
#endif /* RPM_H */