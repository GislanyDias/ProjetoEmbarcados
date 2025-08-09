#include "ssd1306.h"
#include "font.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "SSD1306";

// Display buffer
static uint8_t display_buffer[SSD1306_BUFFER_SIZE];

// Function to send command to SSD1306
esp_err_t ssd1306_write_command(uint8_t cmd) {
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SSD1306_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, 0x00, true);  // Control byte for command
    i2c_master_write_byte(cmd_handle, cmd, true);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
    return ret;
}

// Function to send data to SSD1306
esp_err_t ssd1306_write_data(uint8_t* data, size_t data_len) {
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SSD1306_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, 0x40, true);  // Control byte for data
    i2c_master_write(cmd_handle, data, data_len, true);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
    return ret;
}

// Initialize SSD1306 display
esp_err_t ssd1306_init(void) {
    esp_err_t ret;
    
    // Initialization sequence
    ret = ssd1306_write_command(0xAE);  // Display off
    ret |= ssd1306_write_command(0xD5);  // Set display clock div
    ret |= ssd1306_write_command(0x80);  // Suggested ratio
    ret |= ssd1306_write_command(0xA8);  // Set multiplex
    ret |= ssd1306_write_command(0x3F);  // 1/64 duty
    ret |= ssd1306_write_command(0xD3);  // Set display offset
    ret |= ssd1306_write_command(0x00);  // No offset
    ret |= ssd1306_write_command(0x40);  // Set start line
    ret |= ssd1306_write_command(0x8D);  // Charge pump
    ret |= ssd1306_write_command(0x14);  // Enable charge pump
    ret |= ssd1306_write_command(0x20);  // Memory mode
    ret |= ssd1306_write_command(0x00);  // Horizontal addressing
    ret |= ssd1306_write_command(0xA1);  // Segment remap
    ret |= ssd1306_write_command(0xC8);  // COM output scan direction
    ret |= ssd1306_write_command(0xDA);  // Set COM pins
    ret |= ssd1306_write_command(0x12);  // Alternative COM pin config
    ret |= ssd1306_write_command(0x81);  // Set contrast
    ret |= ssd1306_write_command(0xCF);  // 
    ret |= ssd1306_write_command(0xD9);  // Set precharge
    ret |= ssd1306_write_command(0xF1);  // 
    ret |= ssd1306_write_command(0xDB);  // Set vcom detect
    ret |= ssd1306_write_command(0x40);  // 
    ret |= ssd1306_write_command(0xA4);  // Display resume
    ret |= ssd1306_write_command(0xA6);  // Normal display
    ret |= ssd1306_write_command(0x2E);  // Deactivate scroll
    ret |= ssd1306_write_command(0xAF);  // Display on
    
    if (ret == ESP_OK) {
        ssd1306_clear_buffer();
        ssd1306_display_buffer();
        ESP_LOGI(TAG, "Display initialized successfully");
    } else {
        ESP_LOGE(TAG, "Display initialization failed");
    }
    
    return ret;
}

// Clear display buffer
void ssd1306_clear_buffer(void) {
    memset(display_buffer, 0, SSD1306_BUFFER_SIZE);
}

// Send buffer to display
esp_err_t ssd1306_display_buffer(void) {
    return ssd1306_write_data(display_buffer, SSD1306_BUFFER_SIZE);
}

// Alias for compatibility
esp_err_t ssd1306_update_display(void) {
    return ssd1306_display_buffer();
}

// Set pixel in buffer
void ssd1306_set_pixel(int x, int y, bool color) {
    if (x >= 0 && x < SSD1306_WIDTH && y >= 0 && y < SSD1306_HEIGHT) {
        int page = y / 8;
        int bit = y % 8;
        int index = x + page * SSD1306_WIDTH;
        
        if (color) {
            display_buffer[index] |= (1 << bit);
        } else {
            display_buffer[index] &= ~(1 << bit);
        }
    }
}

// Draw character from font
void ssd1306_draw_char(int x, int y, char c) {
    if (c >= 32 && c <= 126) {
        int char_index = c - 32;
        for (int row = 0; row < 8; row++) {
            uint8_t row_data = font8x8_basic[char_index][row];
            for (int col = 0; col < 8; col++) {
                if (row_data & (1 << col)) {
                    ssd1306_set_pixel(x + col, y + row, true);
                }
            }
        }
    }
}

// Draw string
void ssd1306_draw_string(int x, int y, const char* str) {
    int pos_x = x;
    while (*str) {
        ssd1306_draw_char(pos_x, y, *str);
        pos_x += 8;
        str++;
        if (pos_x >= SSD1306_WIDTH) break;
    }
}

// Draw line using Bresenham's algorithm
void ssd1306_draw_line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int x = x0;
    int y = y0;
    int n = 1 + dx + dy;
    int x_inc = (x1 > x0) ? 1 : -1;
    int y_inc = (y1 > y0) ? 1 : -1;
    int error = dx - dy;
    
    dx *= 2;
    dy *= 2;
    
    for (; n > 0; --n) {
        ssd1306_set_pixel(x, y, true);
        
        if (error > 0) {
            x += x_inc;
            error -= dy;
        } else {
            y += y_inc;
            error += dx;
        }
    }
}

// Draw rectangle (filled or outline)
void ssd1306_draw_rect(int x, int y, int width, int height, bool filled) {
    if (filled) {
        for (int i = x; i < x + width; i++) {
            for (int j = y; j < y + height; j++) {
                ssd1306_set_pixel(i, j, true);
            }
        }
    } else {
        ssd1306_draw_line(x, y, x + width - 1, y);
        ssd1306_draw_line(x, y + height - 1, x + width - 1, y + height - 1);
        ssd1306_draw_line(x, y, x, y + height - 1);
        ssd1306_draw_line(x + width - 1, y, x + width - 1, y + height - 1);
    }
}

// Draw circle using midpoint circle algorithm
void ssd1306_draw_circle(int x0, int y0, int radius, bool filled) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        if (filled) {
            ssd1306_draw_line(x0 - x, y0 + y, x0 + x, y0 + y);
            ssd1306_draw_line(x0 - x, y0 - y, x0 + x, y0 - y);
            ssd1306_draw_line(x0 - y, y0 + x, x0 + y, y0 + x);
            ssd1306_draw_line(x0 - y, y0 - x, x0 + y, y0 - x);
        } else {
            ssd1306_set_pixel(x0 + x, y0 + y, true);
            ssd1306_set_pixel(x0 + y, y0 + x, true);
            ssd1306_set_pixel(x0 - y, y0 + x, true);
            ssd1306_set_pixel(x0 - x, y0 + y, true);
            ssd1306_set_pixel(x0 - x, y0 - y, true);
            ssd1306_set_pixel(x0 - y, y0 - x, true);
            ssd1306_set_pixel(x0 + y, y0 - x, true);
            ssd1306_set_pixel(x0 + x, y0 - y, true);
        }

        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}