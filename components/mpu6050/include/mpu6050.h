#ifndef MPU6050_H
#define MPU6050_H

#include "driver/i2c.h"
#include "esp_log.h"

#define MPU6050_I2C_ADDRESS         0x68
#define MPU6050_PWR_MGMT_1          0x6B
#define MPU6050_GYRO_CONFIG         0x1B
#define MPU6050_ACCEL_CONFIG        0x1C
#define MPU6050_ACCEL_XOUT_H        0x3B
#define MPU6050_GYRO_XOUT_H         0x43

typedef struct {
    float gyro_offset_x;
    float accel_offset_x;
    float accel_offset_y;
    bool gyro_calibrated;
} mpu6050_dev_t;

esp_err_t mpu6050_init(mpu6050_dev_t *dev);
void mpu6050_calibrate_improved(mpu6050_dev_t *dev);
esp_err_t mpu6050_read_gyro(mpu6050_dev_t *dev, float *gyro_x, float *gyro_y, float *gyro_z);
esp_err_t mpu6050_read_accel(mpu6050_dev_t *dev, float *accel_x, float *accel_y, float *accel_z);

#endif