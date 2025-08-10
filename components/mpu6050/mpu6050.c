#include "mpu6050.h"
#include <math.h>

static const char *TAG = "MPU6050";

// Função para escrever um byte no MPU6050
static esp_err_t mpu6050_write_byte(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (MPU6050_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, reg, true);
    i2c_master_write_byte(cmd_handle, data, true);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
    return ret;
}

// Função para ler bytes do MPU6050
static esp_err_t mpu6050_read_bytes(uint8_t reg, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (MPU6050_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, reg, true);
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (MPU6050_I2C_ADDRESS << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd_handle, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd_handle, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
    return ret;
}

// Inicialização do MPU6050
esp_err_t mpu6050_init(mpu6050_dev_t *dev) {
    esp_err_t ret;
    
    // Inicializar estrutura do dispositivo
    dev->gyro_offset_x = 0.0f;
    dev->accel_offset_x = 0.0f;
    dev->accel_offset_y = 0.0f;
    dev->gyro_calibrated = false;
    
    // Despertar o MPU6050 (sai do modo sleep)
    ret = mpu6050_write_byte(MPU6050_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake up MPU6050");
        return ret;
    }
    
    // Configurar giroscópio para ±250°/s (sensibilidade: 131 LSB/°/s)
    ret = mpu6050_write_byte(MPU6050_GYRO_CONFIG, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure gyroscope");
        return ret;
    }
    
    // Configurar acelerômetro para ±2g (sensibilidade: 16384 LSB/g)
    ret = mpu6050_write_byte(MPU6050_ACCEL_CONFIG, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure accelerometer");
        return ret;
    }
    
    ESP_LOGI(TAG, "MPU6050 initialized successfully");
    return ESP_OK;
}

// Função para calibrar o giroscópio
void mpu6050_calibrate_improved(mpu6050_dev_t *dev) {
    ESP_LOGI(TAG, "Starting MPU6050 calibration...");
    
    float gyro_sum_x = 0.0f;
    float accel_sum_x = 0.0f;
    float accel_sum_y = 0.0f;
    int samples = 200;  // More samples for better calibration
    
    for (int i = 0; i < samples; i++) {
        uint8_t gyro_data[6];
        uint8_t accel_data[6];
        
        // Read gyroscope
        if (mpu6050_read_bytes(MPU6050_GYRO_XOUT_H, gyro_data, 6) == ESP_OK) {
            int16_t gyro_x_raw = (int16_t)((gyro_data[0] << 8) | gyro_data[1]);
            float gyro_x = (float)gyro_x_raw / 131.0f;
            gyro_sum_x += gyro_x;
        }
        
        // Read accelerometer
        if (mpu6050_read_bytes(MPU6050_ACCEL_XOUT_H, accel_data, 6) == ESP_OK) {
            int16_t accel_x_raw = (int16_t)((accel_data[0] << 8) | accel_data[1]);
            int16_t accel_y_raw = (int16_t)((accel_data[2] << 8) | accel_data[3]);
            
            float accel_x = (float)accel_x_raw / 16384.0f;
            float accel_y = (float)accel_y_raw / 16384.0f;
            
            accel_sum_x += accel_x;
            accel_sum_y += accel_y;
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
    dev->gyro_offset_x = gyro_sum_x / samples;
    dev->accel_offset_x = accel_sum_x / samples;
    dev->accel_offset_y = accel_sum_y / samples;
    dev->gyro_calibrated = true;
    
    ESP_LOGI(TAG, "Calibration completed");
}

// Função para ler dados do giroscópio
esp_err_t mpu6050_read_gyro(mpu6050_dev_t *dev, float *gyro_x, float *gyro_y, float *gyro_z) {
    uint8_t data[6];
    esp_err_t ret = mpu6050_read_bytes(MPU6050_GYRO_XOUT_H, data, 6);
    
    if (ret == ESP_OK) {
        int16_t gyro_x_raw = (int16_t)((data[0] << 8) | data[1]);
        int16_t gyro_y_raw = (int16_t)((data[2] << 8) | data[3]);
        int16_t gyro_z_raw = (int16_t)((data[4] << 8) | data[5]);
        
        // Convert to °/s (131 LSB/°/s for ±250°/s)
        *gyro_x = ((float)gyro_x_raw / 131.0f) - dev->gyro_offset_x;
        *gyro_y = (float)gyro_y_raw / 131.0f;
        *gyro_z = (float)gyro_z_raw / 131.0f;
    }
    
    return ret;
}

// Função para ler dados do acelerômetro
esp_err_t mpu6050_read_accel(mpu6050_dev_t *dev, float *accel_x, float *accel_y, float *accel_z) {
    uint8_t data[6];
    esp_err_t ret = mpu6050_read_bytes(MPU6050_ACCEL_XOUT_H, data, 6);
    
    if (ret == ESP_OK) {
        int16_t accel_x_raw = (int16_t)((data[0] << 8) | data[1]);
        int16_t accel_y_raw = (int16_t)((data[2] << 8) | data[3]);
        int16_t accel_z_raw = (int16_t)((data[4] << 8) | data[5]);
        
        // Convert to g (16384 LSB/g for ±2g)
        *accel_x = ((float)accel_x_raw / 16384.0f) - dev->accel_offset_x;
        *accel_y = ((float)accel_y_raw / 16384.0f) - dev->accel_offset_y;
        *accel_z = (float)accel_z_raw / 16384.0f;
    }
    
    return ret;
}