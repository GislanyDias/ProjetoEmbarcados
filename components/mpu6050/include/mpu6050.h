#ifndef MPU6050_H
#define MPU6050_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "i2clib.h"

#define MPU6050_PWR_MGMT_1          0x6B    
#define MPU6050_SMPLRT_DIV          0x19   
#define MPU6050_CONFIG              0x1A   
#define MPU6050_GYRO_CONFIG         0x1B    
#define MPU6050_ACCEL_CONFIG        0x1C   
#define MPU6050_WHO_AM_I            0x75   

#define MPU6050_ACCEL_XOUT_H        0x3B
#define MPU6050_ACCEL_XOUT_L        0x3C
#define MPU6050_ACCEL_YOUT_H        0x3D
#define MPU6050_ACCEL_YOUT_L        0x3E
#define MPU6050_ACCEL_ZOUT_H        0x3F
#define MPU6050_ACCEL_ZOUT_L        0x40
#define MPU6050_TEMP_OUT_H          0x41
#define MPU6050_TEMP_OUT_L          0x42
#define MPU6050_GYRO_XOUT_H         0x43
#define MPU6050_GYRO_XOUT_L         0x44
#define MPU6050_GYRO_YOUT_H         0x45
#define MPU6050_GYRO_YOUT_L         0x46
#define MPU6050_GYRO_ZOUT_H         0x47
#define MPU6050_GYRO_ZOUT_L         0x48

typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} mpu6050_data_t;

esp_err_t i2c_master_init(void);
esp_err_t mpu6050_write_byte(uint8_t reg_addr, uint8_t data);
esp_err_t mpu6050_read_byte(uint8_t reg_addr, uint8_t *data);
esp_err_t mpu6050_read_bytes(uint8_t reg_addr, uint8_t *data, size_t len);
esp_err_t mpu6050_init(void);
esp_err_t mpu6050_read_all(mpu6050_data_t *data);
void mpu6050_convert_data(mpu6050_data_t *raw_data, float *accel_g, float *gyro_dps, float *temp_c);
void mpu6050_task(void *pvParameters);
esp_err_t mpu6050_read_acceleration(float* ax, float* ay, float* az);

#endif