#include "ssd.h"
#include <string.h>

// Etiqueta para logs
static const char *OLED_LOG_TAG = "OLED_DISPLAY";

// Buffer de pixels na memória
static uint8_t pixel_buffer[OLED_DISPLAY_WIDTH * OLED_PAGE_COUNT];

// --- Funções Estáticas de Comunicação ---

/**
 * @brief Envia um comando para o display OLED via I2C.
 * @param command O comando a ser enviado.
 * @return ESP_OK em caso de sucesso, caso contrário um código de erro.
 */
static esp_err_t oled_send_command(uint8_t command) {
    i2c_cmd_handle_t command_link = i2c_cmd_link_create();
    i2c_master_start(command_link);
    i2c_master_write_byte(command_link, (OLED_DEVICE_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(command_link, 0x00, true); // Co = 0, D/C = 0
    i2c_master_write_byte(command_link, command, true);
    i2c_master_stop(command_link);
    esp_err_t result = i2c_master_cmd_begin(I2C_BUS_PORT, command_link, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(command_link);
    return result;
}

/**
 * @brief Envia um bloco de dados para o display OLED via I2C.
 * @param data O array de dados a ser enviado.
 * @param len O tamanho do array de dados.
 * @return ESP_OK em caso de sucesso, caso contrário um código de erro.
 */
static esp_err_t oled_send_data(uint8_t *data, size_t len) {
    i2c_cmd_handle_t command_link = i2c_cmd_link_create();
    i2c_master_start(command_link);
    i2c_master_write_byte(command_link, (OLED_DEVICE_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(command_link, 0x40, true); // Co = 0, D/C = 1
    i2c_master_write(command_link, data, len, true);
    i2c_master_stop(command_link);
    esp_err_t result = i2c_master_cmd_begin(I2C_BUS_PORT, command_link, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(command_link);
    return result;
}

// --- Funções de Driver ---

/**
 * @brief Inicializa o barramento I2C e o display OLED.
 */
void oled_driver_initialize(void) {
    // Configuração do I2C Master
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_OLED_SDA_PIN,
        .scl_io_num = I2C_OLED_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_BUS_FREQ_HZ,
    };
    
    // Instala o driver I2C
    ESP_ERROR_CHECK(i2c_param_config(I2C_BUS_PORT, &config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_BUS_PORT, config.mode, 0, 0, 0));

    // Sequência de inicialização do display SSD1306
    oled_send_command(OLED_CMD_POWER_OFF);
    oled_send_command(OLED_CMD_SET_CLOCK_DIVIDER);
    oled_send_command(0x80);
    oled_send_command(OLED_CMD_SET_MULTIPLEX_RATIO);
    oled_send_command(OLED_DISPLAY_HEIGHT - 1);
    oled_send_command(OLED_CMD_SET_DISPLAY_OFFSET);
    oled_send_command(0x00);
    oled_send_command(OLED_CMD_SET_START_LINE);
    oled_send_command(OLED_CMD_CHARGE_PUMP_SET);
    oled_send_command(0x14);
    oled_send_command(OLED_CMD_PAGE_ADDRESSING);
    oled_send_command(0x00);
    oled_send_command(OLED_CMD_SEGMENT_REMAP);
    oled_send_command(OLED_CMD_SCAN_DIRECTION_DEC);
    oled_send_command(OLED_CMD_SET_COM_PINS);
    oled_send_command(0x12);
    oled_send_command(OLED_CMD_SET_BRIGHTNESS);
    oled_send_command(0xCF);
    oled_send_command(OLED_CMD_SET_PRECHARGE_PERIOD);
    oled_send_command(0xF1);
    oled_send_command(OLED_CMD_VCOMH_DESELECT);
    oled_send_command(0x40);
    oled_send_command(OLED_CMD_NORMAL_MODE);
    
    // Limpa o buffer de pixels e atualiza o display
    oled_clear_buffer();
    oled_refresh_display();
    
    oled_send_command(OLED_CMD_POWER_ON);
    ESP_LOGI(OLED_LOG_TAG, "Display OLED inicializado com sucesso.");
}

/**
 * @brief Limpa o buffer de pixels na memória.
 */
void oled_clear_buffer(void) {
    memset(pixel_buffer, 0, sizeof(pixel_buffer));
}

/**
 * @brief Envia o conteúdo do buffer para o display.
 */
void oled_refresh_display(void) {
    // Define o modo de endereçamento de página e envia o buffer completo
    oled_send_command(OLED_CMD_COLUMN_ADDRESS_SET);
    oled_send_command(0);
    oled_send_command(OLED_DISPLAY_WIDTH - 1);
    oled_send_command(OLED_CMD_PAGE_ADDRESS_SET);
    oled_send_command(0);
    oled_send_command(OLED_PAGE_COUNT - 1);
    
    oled_send_data(pixel_buffer, sizeof(pixel_buffer));
}

// --- Funções de Desenho ---

/**
 * @brief Define o estado de um pixel (ligado ou desligado) no buffer.
 * @param x_coord Coordenada X do pixel.
 * @param y_coord Coordenada Y do pixel.
 * @param state Estado do pixel (true para ligado, false para desligado).
 */
void oled_set_pixel_state(int x_coord, int y_coord, bool state) {
    if (x_coord >= 0 && x_coord < OLED_DISPLAY_WIDTH && y_coord >= 0 && y_coord < OLED_DISPLAY_HEIGHT) {
        int index = x_coord + (y_coord / 8) * OLED_DISPLAY_WIDTH;
        if (state) {
            pixel_buffer[index] |= (1 << (y_coord % 8));
        } else {
            pixel_buffer[index] &= ~(1 << (y_coord % 8));
        }
    }
}

/**
 * @brief Desenha um caractere no buffer.
 * @param x_pos Coordenada X inicial.
 * @param y_pos Coordenada Y inicial.
 * @param character O caractere a ser desenhado.
 */
void oled_draw_character(int x_pos, int y_pos, char character) {
    if (character < 32 || character > 126) character = 32; // Espaço para caracteres inválidos
    const uint8_t *char_map = font8x8[character - 32];
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            bool pixel_on = (char_map[i] >> j) & 1;
            oled_set_pixel_state(x_pos + j, y_pos + i, pixel_on);
        }
    }
}

/**
 * @brief Desenha uma string no buffer.
 * @param x_pos Coordenada X inicial.
 * @param y_pos Coordenada Y inicial.
 * @param text A string a ser desenhada.
 */
void oled_draw_string(int x_pos, int y_pos, const char* text) {
    int current_x = x_pos;
    while (*text) {
        oled_draw_character(current_x, y_pos, *text);
        current_x += 8;
        text++;
    }
}

/**
 * @brief Desenha uma linha no buffer.
 * @param x1 Coordenada X de início.
 * @param y1 Coordenada Y de início.
 * @param x2 Coordenada X de fim.
 * @param y2 Coordenada Y de fim.
 */
void oled_draw_line(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        oled_set_pixel_state(x1, y1, true);
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

/**
 * @brief Desenha um retângulo no buffer.
 * @param x_pos Coordenada X de início.
 * @param y_pos Coordenada Y de início.
 * @param width Largura do retângulo.
 * @param height Altura do retângulo.
 * @param fill Preencher o retângulo (true) ou apenas o contorno (false).
 */
void oled_draw_rectangle(int x_pos, int y_pos, int width, int height, bool fill) {
    if (fill) {
        for (int y = y_pos; y < y_pos + height; y++) {
            for (int x = x_pos; x < x_pos + width; x++) {
                oled_set_pixel_state(x, y, true);
            }
        }
    } else {
        oled_draw_line(x_pos, y_pos, x_pos + width - 1, y_pos);
        oled_draw_line(x_pos + width - 1, y_pos, x_pos + width - 1, y_pos + height - 1);
        oled_draw_line(x_pos + width - 1, y_pos + height - 1, x_pos, y_pos + height - 1);
        oled_draw_line(x_pos, y_pos + height - 1, x_pos, y_pos);
    }
}
