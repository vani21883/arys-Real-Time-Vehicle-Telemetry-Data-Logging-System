#ifndef IMU_H
#define IMU_H

#include <stdint.h>
#include "esp_err.h"

/* MPU6050 I2C address */
#define MPU6050_I2C_ADDR    0x68

/* Raw sensor data */
typedef struct {
    float accel_x, accel_y, accel_z;   /* m/s^2 */
    float gyro_x,  gyro_y,  gyro_z;    /* deg/s */
    float temp_c;
} imu_data_t;

esp_err_t   imu_init(void);
esp_err_t   imu_read(imu_data_t *out);
void        imu_task(void *pvParameters);

#endif /* IMU_H */