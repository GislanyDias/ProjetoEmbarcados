#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c.h"

#define SSD1306_I2C_ADDRESS         0x3C
#define SSD1306_WIDTH               128
#define SSD1306_HEIGHT              64
#define SSD1306_BUFFER_SIZE         1024

typedef struct {
    i2c_port_t i2c_num;
    uint8_t buffer[SSD1306_BUFFER_SIZE];
} ssd1306_t;

esp_err_t ssd1306_init(ssd1306_t dev, i2c_port_t i2c_num);
void ssd1306_clear_buffer(ssd1306_tdev);
esp_err_t ssd1306_display_buffer(ssd1306_t dev);
void ssd1306_set_pixel(ssd1306_tdev, int x, int y, bool color);
void ssd1306_draw_char(ssd1306_t dev, int x, int y, char c);
void ssd1306_draw_string(ssd1306_tdev, int x, int y, const char* str);
void ssd1306_draw_line(ssd1306_t dev, int x0, int y0, int x1, int y1);
void ssd1306_draw_rect(ssd1306_tdev, int x, int y, int width, int height, bool filled);

#endif