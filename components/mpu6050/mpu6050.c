#include "mpu6050.h"

static const char *TAG = "MPU6050";
#define I2C_MASTER_NUM I2C_NUM_0

// Inicializa o barramento I2C
esp_err_t mpu6050_i2c_init(void)
{
    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = MPU6050_I2C_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = MPU6050_I2C_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = MPU6050_I2C_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(MPU6050_I2C_PORT, &i2c_cfg);
    if (err != ESP_OK) return err;

    return i2c_driver_install(MPU6050_I2C_PORT, i2c_cfg.mode,
                              MPU6050_I2C_RX_BUF_DISABLE,
                              MPU6050_I2C_TX_BUF_DISABLE, 0);
}

// Escreve um registrador
esp_err_t mpu6050_write_reg(uint8_t reg_addr, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(MPU6050_I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Lê um registrador
esp_err_t mpu6050_read_reg(uint8_t reg_addr, uint8_t *value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, value, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(MPU6050_I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Lê múltiplos registradores
esp_err_t mpu6050_read_regs(uint8_t start_addr, uint8_t *buffer, size_t length)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, start_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_I2C_ADDR << 1) | I2C_MASTER_READ, true);

    if (length > 1) {
        i2c_master_read(cmd, buffer, length - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, buffer + length - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(MPU6050_I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Inicializa o sensor MPU6050
esp_err_t mpu6050_init(void)
{
    uint8_t device_id;
    esp_err_t ret = mpu6050_read_reg(MPU6050_REG_WHO_AM_I, &device_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao ler WHO_AM_I: %s", esp_err_to_name(ret));
        return ret;
    }

    if (device_id != MPU6050_I2C_ADDR) {
        ESP_LOGE(TAG, "Dispositivo não reconhecido: WHO_AM_I = 0x%02X", device_id);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "MPU6050 detectado: WHO_AM_I = 0x%02X", device_id);

    if ((ret = mpu6050_write_reg(MPU6050_REG_PWR_MGMT_1, 0x00)) != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(100));

    if ((ret = mpu6050_write_reg(MPU6050_REG_SMPLRT_DIV, 0x07)) != ESP_OK) return ret;
    if ((ret = mpu6050_write_reg(MPU6050_REG_CONFIG, 0x00)) != ESP_OK) return ret;
    if ((ret = mpu6050_write_reg(MPU6050_REG_GYRO_CONFIG, 0x00)) != ESP_OK) return ret;
    if ((ret = mpu6050_write_reg(MPU6050_REG_ACCEL_CONFIG, 0x00)) != ESP_OK) return ret;

    ESP_LOGI(TAG, "MPU6050 inicializado com sucesso");
    return ESP_OK;
}

// Lê os dados brutos do sensor
esp_err_t mpu6050_read_raw(mpu6050_raw_data_t *raw)
{
    uint8_t raw_buf[14];
    esp_err_t ret = mpu6050_read_regs(MPU6050_REG_ACCEL_XOUT_H, raw_buf, 14);
    if (ret != ESP_OK) return ret;

    raw->accel_x = (int16_t)((raw_buf[0] << 8) | raw_buf[1]);
    raw->accel_y = (int16_t)((raw_buf[2] << 8) | raw_buf[3]);
    raw->accel_z = (int16_t)((raw_buf[4] << 8) | raw_buf[5]);
    raw->temp    = (int16_t)((raw_buf[6] << 8) | raw_buf[7]);
    raw->gyro_x  = (int16_t)((raw_buf[8] << 8) | raw_buf[9]);
    raw->gyro_y  = (int16_t)((raw_buf[10] << 8) | raw_buf[11]);
    raw->gyro_z  = (int16_t)((raw_buf[12] << 8) | raw_buf[13]);

    return ESP_OK;
}

// Converte dados brutos para unidades físicas
void mpu6050_convert(const mpu6050_raw_data_t *raw, float *accel_g, float *gyro_dps, float *temp_c)
{
    accel_g[0] = raw->accel_x / 16384.0f;
    accel_g[1] = raw->accel_y / 16384.0f;
    accel_g[2] = raw->accel_z / 16384.0f;

    gyro_dps[0] = raw->gyro_x / 131.0f;
    gyro_dps[1] = raw->gyro_y / 131.0f;
    gyro_dps[2] = raw->gyro_z / 131.0f;

    *temp_c = raw->temp / 340.0f + 36.53f;
}

// Task de leitura contínua
void mpu6050_task(void *pvParameters)
{
    mpu6050_raw_data_t raw_data;
    float accel[3], gyro[3], temp;

    while (true) {
        if (mpu6050_read_raw(&raw_data) == ESP_OK) {
            mpu6050_convert(&raw_data, accel, gyro, &temp);

            ESP_LOGI(TAG, "Accel [g]: X=%.2f Y=%.2f Z=%.2f", accel[0], accel[1], accel[2]);
            ESP_LOGI(TAG, "Gyro [°/s]: X=%.2f Y=%.2f Z=%.2f", gyro[0], gyro[1], gyro[2]);
            ESP_LOGI(TAG, "Temp: %.2f°C", temp);
        } else {
            ESP_LOGE(TAG, "Erro ao ler sensor");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
