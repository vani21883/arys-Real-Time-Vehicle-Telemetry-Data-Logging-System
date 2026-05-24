#include "imu.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_log.h"

#include "shared_data.h"
#include "freertos/semphr.h"
#include "esp_timer.h"

#define TAG "IMU"

/* I2C Configuration */
#define I2C_MASTER_PORT     I2C_NUM_0
#define I2C_SDA_PIN         21
#define I2C_SCL_PIN         22
#define I2C_FREQ_HZ         400000

/* MPU6050 Registers */
#define MPU6050_REG_PWR_MGMT_1     0x6B
#define MPU6050_REG_SMPLRT_DIV     0x19
#define MPU6050_REG_CONFIG         0x1A
#define MPU6050_REG_GYRO_CONFIG    0x1B
#define MPU6050_REG_ACCEL_CONFIG   0x1C
#define MPU6050_REG_WHO_AM_I       0x75
#define MPU6050_REG_ACCEL_XOUT_H   0x3B

/* Sensitivity Scale Factors */
#define ACCEL_SCALE_2G     16384.0f
#define GYRO_SCALE_250DPS  131.0f

/* Gravity */
#define GRAVITY_MS2        9.80665f

static i2c_master_bus_handle_t i2c_bus_handle;
static i2c_master_dev_handle_t imu_dev_handle;

/* ---------- Helper Functions ---------- */

static esp_err_t imu_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = {reg, value};

    return i2c_master_transmit(
        imu_dev_handle,
        data,
        sizeof(data),
        -1
    );
}

static esp_err_t imu_read_regs(uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(
        imu_dev_handle,
        &reg,
        1,
        data,
        len,
        -1
    );
}

/* ---------- IMU Initialization ---------- */

esp_err_t imu_init(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Initializing MPU6050...");

    /* Configure I2C master bus */
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_PORT,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ret = i2c_new_master_bus(&bus_config, &i2c_bus_handle);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C bus");
        return ret;
    }

    /* Configure MPU6050 device */
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MPU6050_I2C_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };

    ret = i2c_master_bus_add_device(
        i2c_bus_handle,
        &dev_config,
        &imu_dev_handle
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add MPU6050 device");
        return ret;
    }

    /* Verify device */
    uint8_t who_am_i = 0;

    ret = imu_read_regs(
        MPU6050_REG_WHO_AM_I,
        &who_am_i,
        1
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read WHO_AM_I");
        return ret;
    }

    ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", who_am_i);

    if (who_am_i != 0x68) {
        ESP_LOGE(TAG, "MPU6050 not detected!");
        return ESP_FAIL;
    }

    /* Wake sensor */
    ret = imu_write_reg(
        MPU6050_REG_PWR_MGMT_1,
        0x00
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake MPU6050");
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    /* Set sample rate divider */
    ret = imu_write_reg(
        MPU6050_REG_SMPLRT_DIV,
        0x07
    );

    if (ret != ESP_OK) {
        return ret;
    }

    /* Configure DLPF */
    ret = imu_write_reg(
        MPU6050_REG_CONFIG,
        0x03
    );

    if (ret != ESP_OK) {
        return ret;
    }

    /* Gyro ±250 deg/s */
    ret = imu_write_reg(
        MPU6050_REG_GYRO_CONFIG,
        0x00
    );

    if (ret != ESP_OK) {
        return ret;
    }

    /* Accel ±2g */
    ret = imu_write_reg(
        MPU6050_REG_ACCEL_CONFIG,
        0x00
    );

    if (ret != ESP_OK) {
        return ret;
    }

    ESP_LOGI(TAG, "MPU6050 initialized successfully");

    return ESP_OK;
}

/* ---------- Read Sensor Data ---------- */

esp_err_t imu_read(imu_data_t *out)
{
    if (out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t raw_data[14];

    esp_err_t ret = imu_read_regs(
        MPU6050_REG_ACCEL_XOUT_H,
        raw_data,
        sizeof(raw_data)
    );

    if (ret != ESP_OK) {
        return ret;
    }

    /* Parse raw values */
    int16_t accel_x_raw =
        (int16_t)((raw_data[0] << 8) | raw_data[1]);

    int16_t accel_y_raw =
        (int16_t)((raw_data[2] << 8) | raw_data[3]);

    int16_t accel_z_raw =
        (int16_t)((raw_data[4] << 8) | raw_data[5]);

    int16_t temp_raw =
        (int16_t)((raw_data[6] << 8) | raw_data[7]);

    int16_t gyro_x_raw =
        (int16_t)((raw_data[8] << 8) | raw_data[9]);

    int16_t gyro_y_raw =
        (int16_t)((raw_data[10] << 8) | raw_data[11]);

    int16_t gyro_z_raw =
        (int16_t)((raw_data[12] << 8) | raw_data[13]);

    /* Convert accelerometer to m/s² */
    out->accel_x =
        ((float)accel_x_raw / ACCEL_SCALE_2G) * GRAVITY_MS2;

    out->accel_y =
        ((float)accel_y_raw / ACCEL_SCALE_2G) * GRAVITY_MS2;

    out->accel_z =
        ((float)accel_z_raw / ACCEL_SCALE_2G) * GRAVITY_MS2;

    /* Convert gyro to deg/s */
    out->gyro_x =
        (float)gyro_x_raw / GYRO_SCALE_250DPS;

    out->gyro_y =
        (float)gyro_y_raw / GYRO_SCALE_250DPS;

    out->gyro_z =
        (float)gyro_z_raw / GYRO_SCALE_250DPS;

    /* Convert temperature */
    out->temp_c =
        ((float)temp_raw / 340.0f) + 36.53f;

    return ESP_OK;
}

/* ---------- IMU Task ---------- */

void imu_task(void *pvParameters)
{
    imu_data_t data;

    while (1) {

        if (imu_read(&data) == ESP_OK) {

            /* -------------------------------------- */
            /* Update Shared Telemetry */
            /* -------------------------------------- */

            xSemaphoreTake(
                g_telemetry_mutex,
                portMAX_DELAY
            );

            g_telemetry_data.accel_x = data.accel_x;
            g_telemetry_data.accel_y = data.accel_y;
            g_telemetry_data.accel_z = data.accel_z;

            g_telemetry_data.gyro_x = data.gyro_x;
            g_telemetry_data.gyro_y = data.gyro_y;
            g_telemetry_data.gyro_z = data.gyro_z;

            g_telemetry_data.imu_temp_c = data.temp_c;

            g_telemetry_data.timestamp_ms =
                esp_timer_get_time() / 1000;

            g_telemetry_data.imu_last_update_ms = esp_timer_get_time() / 1000;

            xSemaphoreGive(g_telemetry_mutex);

            /* -------------------------------------- */
            /* Debug Logging */
            /* -------------------------------------- */

            ESP_LOGI(
                TAG,
                "ACCEL [%.2f %.2f %.2f] m/s² | "
                "GYRO [%.2f %.2f %.2f] dps | "
                "TEMP %.2f C",

                data.accel_x,
                data.accel_y,
                data.accel_z,

                data.gyro_x,
                data.gyro_y,
                data.gyro_z,

                data.temp_c
            );
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/*void imu_task(void *pvParameters)
{
    imu_data_t data;

    while (1) {

        if (imu_read(&data) == ESP_OK) {

            ESP_LOGI(
                TAG,
                "ACCEL [%.2f %.2f %.2f] m/s² | "
                "GYRO [%.2f %.2f %.2f] dps | "
                "TEMP %.2f C",
                data.accel_x,
                data.accel_y,
                data.accel_z,
                data.gyro_x,
                data.gyro_y,
                data.gyro_z,
                data.temp_c
            );
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
} */



/*
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
    printf("[IMU] init placeholder\n");
    return ESP_OK;
}

esp_err_t imu_read(imu_data_t *out)
{
    (void)out;
    return ESP_OK;
}

void imu_task(void *pvParameters)
{
    imu_data_t data;
    for (;;) {
        imu_read(&data);
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}
*/