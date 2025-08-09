#ifndef _MPU6050_H_
#define _MPU6050_H_

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"

// Endereço I2C e registradores do MPU6050
#define MPU6050_I2C_ADDR             0x68

#define MPU6050_REG_PWR_MGMT_1      0x6B
#define MPU6050_REG_SMPLRT_DIV      0x19
#define MPU6050_REG_CONFIG          0x1A
#define MPU6050_REG_GYRO_CONFIG     0x1B
#define MPU6050_REG_ACCEL_CONFIG    0x1C
#define MPU6050_REG_WHO_AM_I        0x75

#define MPU6050_REG_ACCEL_XOUT_H    0x3B
#define MPU6050_REG_TEMP_OUT_H      0x41
#define MPU6050_REG_GYRO_XOUT_H     0x43

// Configuração padrão do barramento I2C
#define MPU6050_I2C_SCL_IO          18
#define MPU6050_I2C_SDA_IO          17
#define MPU6050_I2C_PORT            I2C_NUM_0
#define MPU6050_I2C_FREQ_HZ         100000
#define MPU6050_I2C_TX_BUF_DISABLE  0
#define MPU6050_I2C_RX_BUF_DISABLE  0

// Estrutura de dados brutos do MPU6050
typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} mpu6050_raw_data_t;

// Inicialização do barramento I2C
esp_err_t mpu6050_i2c_init(void);

// Escrita e leitura no MPU6050
esp_err_t mpu6050_write_reg(uint8_t reg_addr, uint8_t data);
esp_err_t mpu6050_read_reg(uint8_t reg_addr, uint8_t *data);
esp_err_t mpu6050_read_regs(uint8_t start_addr, uint8_t *data, size_t len);

// Inicialização do MPU6050
esp_err_t mpu6050_init(void);

// Leitura dos dados do sensor
esp_err_t mpu6050_read_raw(mpu6050_raw_data_t *raw_data);

// Conversão de dados brutos para unidades físicas
void mpu6050_convert(const mpu6050_raw_data_t *raw, float *accel_g, float *gyro_dps, float *temp_c);

// Task para leitura contínua do sensor
void mpu6050_task(void *pvParameters);

#endif // _MPU6050_H_
