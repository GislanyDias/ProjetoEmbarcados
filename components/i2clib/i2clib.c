#include "i2clib.h"

typedef enum
{
  I2C_STATE_UNKNOWN = 0,
  I2C_STATE_ACTIVE,
  I2C_STATE_SLEEPING,
  I2C_STATE_STUCK,
  I2C_STATE_ERROR
} i2c_state_t;

static i2c_state_t current_i2c_state = I2C_STATE_UNKNOWN;
static uint32_t i2c_recovery_count = 0;

TaskHandle_t i2c_cleanup_task_handle;

esp_err_t i2c_init(void)
{
  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
  if (ret != ESP_OK)
  {
    return ret;
  }

  ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
  return ret;
}

void i2c_scan(void)
{
  int devices_found = 0;

  for (int addr = 1; addr < 127; addr++)
  {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK)
    {
      devices_found++;
    }
  }
}