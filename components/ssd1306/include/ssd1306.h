#ifndef _SSD1306_H_
#define _SSD1306_H_

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"


#define SSD1306_WIDTH               128
#define SSD1306_HEIGHT              64
#define SSD1306_BUFFER_SIZE         (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

#define SSD1306_I2C_ADDRESS         0x3C


#define I2C_MASTER_NUM              0
#define I2C_MASTER_TIMEOUT_MS       1000


esp_err_t ssd1306_init(void);
void ssd1306_clear_buffer(void);
esp_err_t ssd1306_display_buffer(void);
esp_err_t ssd1306_update_display(void);
void ssd1306_set_pixel(int x, int y, bool color);
void ssd1306_draw_char(int x, int y, char c);
void ssd1306_draw_string(int x, int y, const char* str);
void ssd1306_draw_line(int x0, int y0, int x1, int y1);
void ssd1306_draw_rect(int x, int y, int width, int height, bool filled);
void ssd1306_draw_circle(int x0, int y0, int radius, bool filled);

#endif // _SSD1306_H_