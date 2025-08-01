#ifndef OLED_DRIVER_H
#define OLED_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "font.h" 

// Definições de Hardware I2C
#define I2C_OLED_SCL_PIN        20
#define I2C_OLED_SDA_PIN        21
#define I2C_BUS_PORT            0
#define I2C_BUS_FREQ_HZ         400000

// Definições do Display OLED
#define OLED_DEVICE_ADDRESS     0x3C
#define OLED_DISPLAY_WIDTH      128
#define OLED_DISPLAY_HEIGHT     64
#define OLED_PAGE_COUNT         (OLED_DISPLAY_HEIGHT / 8)

// Macros de Comandos do Display
#define OLED_CMD_POWER_OFF        0xAE
#define OLED_CMD_POWER_ON         0xAF
#define OLED_CMD_SET_BRIGHTNESS   0x81
#define OLED_CMD_NORMAL_MODE      0xA6
#define OLED_CMD_INVERSE_MODE     0xA7
#define OLED_CMD_SET_MULTIPLEX_RATIO 0xA8
#define OLED_CMD_SET_DISPLAY_OFFSET  0xD3
#define OLED_CMD_SET_START_LINE      0x40
#define OLED_CMD_SEGMENT_REMAP       0xA1
#define OLED_CMD_SCAN_DIRECTION_DEC  0xC8
#define OLED_CMD_SET_COM_PINS        0xDA
#define OLED_CMD_SET_CLOCK_DIVIDER   0xD5
#define OLED_CMD_SET_PRECHARGE_PERIOD 0xD9
#define OLED_CMD_VCOMH_DESELECT      0xDB
#define OLED_CMD_CHARGE_PUMP_SET     0x8D
#define OLED_CMD_PAGE_ADDRESSING     0x20
#define OLED_CMD_COLUMN_ADDRESS_SET  0x21
#define OLED_CMD_PAGE_ADDRESS_SET    0x22

// --- Funções de Driver ---

// Inicializa o barramento I2C e o display OLED
void oled_driver_initialize(void);

// Limpa o buffer de pixels na memória
void oled_clear_buffer(void);

// Envia o conteúdo do buffer para o display
void oled_refresh_display(void);

// --- Funções de Desenho ---

// Define o estado de um pixel (ligado ou desligado)
void oled_set_pixel_state(int x_coord, int y_coord, bool state);

// Desenha um caractere no buffer
void oled_draw_character(int x_pos, int y_pos, char character);

// Desenha uma string no buffer
void oled_draw_string(int x_pos, int y_pos, const char* text);

// Desenha uma linha
void oled_draw_line(int x1, int y1, int x2, int y2);

// Desenha um retângulo
void oled_draw_rectangle(int x_pos, int y_pos, int width, int height, bool fill);

#endif // OLED_DRIVER_H
