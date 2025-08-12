#ifndef SSD1306_H
#define SSD1306_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "font.h"
#include "i2clib.h"

#define SSD1306_WIDTH               128
#define SSD1306_HEIGHT              64
#define SSD1306_PAGES               8    
#define SSD1306_FONT_WIDTH          8     
#define SSD1306_WIDTH_IN_CHARS      16   

#define SSD1306_CMD_DISPLAY_OFF     0xAE
#define SSD1306_CMD_DISPLAY_ON      0xAF
#define SSD1306_CMD_SET_CONTRAST    0x81
#define SSD1306_CMD_ENTIRE_DISPLAY_ON 0xA4
#define SSD1306_CMD_NORMAL_DISPLAY  0xA6
#define SSD1306_CMD_INVERT_DISPLAY  0xA7
#define SSD1306_CMD_SET_MULTIPLEX   0xA8
#define SSD1306_CMD_SET_DISPLAY_OFFSET 0xD3
#define SSD1306_CMD_SET_START_LINE  0x40
#define SSD1306_CMD_SEGMENT_REMAP   0xA1
#define SSD1306_CMD_COM_SCAN_DEC    0xC8
#define SSD1306_CMD_SET_COM_PINS    0xDA
#define SSD1306_CMD_SET_CLOCK_DIV   0xD5
#define SSD1306_CMD_SET_PRECHARGE   0xD9
#define SSD1306_CMD_SET_VCOM_DETECT 0xDB
#define SSD1306_CMD_CHARGE_PUMP     0x8D
#define SSD1306_CMD_MEMORY_MODE     0x20
#define SSD1306_CMD_SET_COLUMN_ADDR 0x21
#define SSD1306_CMD_SET_PAGE_ADDR   0x22

esp_err_t ssd1306_write_command(uint8_t cmd);
esp_err_t ssd1306_write_data(uint8_t* data, size_t len);
void ssd1306_init(void);
void ssd1306_clear_buffer(void);
void ssd1306_update_display(void);
void ssd1306_set_pixel(int x, int y, bool on);
void ssd1306_draw_circle_points(int cx, int cy, int x, int y);
void ssd1306_draw_circle(int cx, int cy, int radius, bool filled);
void ssd1306_draw_char(int x, int y, char c);
void ssd1306_draw_string(int x, int y, const char* str);
void ssd1306_draw_line(int x0, int y0, int x1, int y1);
void ssd1306_draw_rect(int x, int y, int w, int h, bool filled);
void ssd1306_test_pattern(void);
int ssd1306_get_string_width(const char *str);

#endif