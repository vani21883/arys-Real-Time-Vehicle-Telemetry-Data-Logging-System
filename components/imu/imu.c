#include "imu.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

#define I2C_MASTER_PORT     I2C_NUM_0
#define I2C_SDA_PIN         21
#define I2C_SCL_PIN         22
#define I2C_FREQ_HZ         400000

esp_err_t imu_init(void)
{
    /* TODO: configure I2C master, wake MPU6050 (write 0x00 to PWR_MGMT_1) */
    printf("[IMU] init placeholder\n");
    return ESP_OK;
}

esp_err_t imu_read(imu_data_t *out)
{
    /* TODO: burst-read 14 bytes from MPU6050 reg 0x3B, parse accel/gyro/temp */
    (void)out;
    return ESP_OK;
}

void imu_task(void *pvParameters)
{
    imu_data_t data;
    for (;;) {
        imu_read(&data);
        /* TODO: push to shared queue */
        vTaskDelay(pdMS_TO_TICKS(10)); /* 100 Hz */
    }
}