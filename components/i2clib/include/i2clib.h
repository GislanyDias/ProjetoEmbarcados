#ifndef I2CLIB_H
#define I2CLIB_H

#include "driver/i2c.h"
#include "esp_event.h"

#define NOTIF_STOP (1UL << 0)

#define I2C_MASTER_SCL_IO           9
#define I2C_MASTER_SDA_IO           8
#define I2C_MASTER_NUM              0   
#define I2C_MASTER_FREQ_HZ          100000 
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_TIMEOUT_MS       1000

#define MPU6050_ADDR                0x68    
#define SSD1306_I2C_ADDR            0x3C  

esp_err_t i2c_init(void);
void i2c_scan(void);
bool i2c_check_bus_active(void);
void i2c_recover_bus(void);
void check_and_recover_i2c_if_needed(void);
esp_err_t check_i2c(void);

#endif