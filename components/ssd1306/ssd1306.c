#include "ssd1306.h"

static const char *TAG = "SSD1306";

static uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / SSD1306_PAGES];

esp_err_t ssd1306_write_command(uint8_t cmd) {
  ESP_LOGD(TAG, "Enviando comando: 0x%02X", cmd);
  i2c_cmd_handle_t cmd_link = i2c_cmd_link_create();
  i2c_master_start(cmd_link);
  i2c_master_write_byte(cmd_link, (SSD1306_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd_link, 0x00, true); // Co = 0, D/C = 0
  i2c_master_write_byte(cmd_link, cmd, true);
  i2c_master_stop(cmd_link);
  esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_link, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd_link);
  
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ERRO ao enviar comando 0x%02X: %s", cmd, esp_err_to_name(ret));
  } else {
    ESP_LOGD(TAG, "Comando 0x%02X enviado com sucesso", cmd);
  }
  return ret;
}

esp_err_t ssd1306_write_data(uint8_t* data, size_t len) {
  ESP_LOGI(TAG, "Enviando %d bytes de dados", len);
  i2c_cmd_handle_t cmd_link = i2c_cmd_link_create();
  i2c_master_start(cmd_link);
  i2c_master_write_byte(cmd_link, (SSD1306_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd_link, 0x40, true); // Co = 0, D/C = 1
  i2c_master_write(cmd_link, data, len, true);
  i2c_master_stop(cmd_link);
  esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_link, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd_link);
  
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ERRO ao enviar dados (%d bytes): %s", len, esp_err_to_name(ret));
  } else {
    ESP_LOGI(TAG, "Dados enviados com sucesso (%d bytes)", len);
    // Log dos primeiros bytes para debug
    ESP_LOGI(TAG, "Primeiros 8 bytes: %02X %02X %02X %02X %02X %02X %02X %02X", 
             data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  }
  return ret;
}

void ssd1306_init(void) {
  ESP_LOGI(TAG, "=== Inicializando SSD1306 ===");
  ESP_LOGI(TAG, "Endereço I2C: 0x%02X", SSD1306_I2C_ADDR);
  ESP_LOGI(TAG, "Resolução: %dx%d", SSD1306_WIDTH, SSD1306_HEIGHT);

  // Sequência de inicialização com logs
  ESP_LOGI(TAG, "Enviando sequência de inicialização...");
  ssd1306_write_command(SSD1306_CMD_DISPLAY_OFF);
  ssd1306_write_command(SSD1306_CMD_SET_CLOCK_DIV);
  ssd1306_write_command(0x80);
  ssd1306_write_command(SSD1306_CMD_SET_MULTIPLEX);
  ssd1306_write_command(0x3F); // 128x64
  ssd1306_write_command(SSD1306_CMD_SET_DISPLAY_OFFSET);
  ssd1306_write_command(0x00);
  ssd1306_write_command(SSD1306_CMD_SET_START_LINE | 0x00);
  ssd1306_write_command(SSD1306_CMD_CHARGE_PUMP);
  ssd1306_write_command(0x14);
  ssd1306_write_command(SSD1306_CMD_MEMORY_MODE);
  ssd1306_write_command(0x00);
  ssd1306_write_command(SSD1306_CMD_SEGMENT_REMAP | 0x01);
  ssd1306_write_command(SSD1306_CMD_COM_SCAN_DEC);
  ssd1306_write_command(SSD1306_CMD_SET_COM_PINS);
  ssd1306_write_command(0x12);
  ssd1306_write_command(SSD1306_CMD_SET_CONTRAST);
  ssd1306_write_command(0xCF);
  ssd1306_write_command(SSD1306_CMD_SET_PRECHARGE);
  ssd1306_write_command(0xF1);
  ssd1306_write_command(SSD1306_CMD_SET_VCOM_DETECT);
  ssd1306_write_command(0x40);
  ssd1306_write_command(SSD1306_CMD_ENTIRE_DISPLAY_ON);
  ssd1306_write_command(SSD1306_CMD_NORMAL_DISPLAY);
  ssd1306_write_command(SSD1306_CMD_DISPLAY_ON);

  vTaskDelay(pdMS_TO_TICKS(10));

  ESP_LOGI(TAG, "=== SSD1306 inicializado ===");
}

void ssd1306_clear_buffer(void) {
  ESP_LOGI(TAG, "Limpando buffer (tamanho: %d bytes)", sizeof(ssd1306_buffer));
  memset(ssd1306_buffer, 0x00, sizeof(ssd1306_buffer));
  ESP_LOGI(TAG, "Buffer limpo!");
}

void ssd1306_update_display(void) {
  ESP_LOGI(TAG, "=== Atualizando display ===");
  
  ESP_LOGI(TAG, "Configurando área de escrita...");
  ssd1306_write_command(SSD1306_CMD_SET_COLUMN_ADDR);
  ssd1306_write_command(0);
  ssd1306_write_command(127);
  ssd1306_write_command(SSD1306_CMD_SET_PAGE_ADDR);
  ssd1306_write_command(0);
  ssd1306_write_command(7);

  ESP_LOGI(TAG, "Enviando buffer para display...");
  esp_err_t ret = ssd1306_write_data(ssd1306_buffer, sizeof(ssd1306_buffer));
  
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "=== Display atualizado com SUCESSO ===");
  } else {
    ESP_LOGE(TAG, "=== ERRO ao atualizar display ===");
  }
}

void ssd1306_test_pattern(void) {
  ESP_LOGI(TAG, "=== Iniciando teste de padrões ===");
  
  // Teste 1: Tela toda branca
  ESP_LOGI(TAG, "Teste 1: Tela toda branca");
  memset(ssd1306_buffer, 0xFF, sizeof(ssd1306_buffer));
  ssd1306_update_display();
  vTaskDelay(pdMS_TO_TICKS(3000));
  
  // Teste 2: Tela toda preta
  ESP_LOGI(TAG, "Teste 2: Tela toda preta");
  memset(ssd1306_buffer, 0x00, sizeof(ssd1306_buffer));
  ssd1306_update_display();
  vTaskDelay(pdMS_TO_TICKS(3000));
  
  // Teste 3: Padrão xadrez
  ESP_LOGI(TAG, "Teste 3: Padrão xadrez");
  for (int i = 0; i < sizeof(ssd1306_buffer); i++) {
    ssd1306_buffer[i] = (i % 2) ? 0xAA : 0x55;
  }
  ssd1306_update_display();
  vTaskDelay(pdMS_TO_TICKS(3000));
  
  // Teste 4: Linhas horizontais
  ESP_LOGI(TAG, "Teste 4: Linhas horizontais");
  for (int i = 0; i < sizeof(ssd1306_buffer); i++) {
    ssd1306_buffer[i] = 0x0F; // 4 pixels ligados, 4 desligados
  }
  ssd1306_update_display();
  vTaskDelay(pdMS_TO_TICKS(3000));
  
  ESP_LOGI(TAG, "=== Fim dos testes ===");
}

void ssd1306_set_pixel(int x, int y, bool on) {
  if (x >= 0 && x < SSD1306_WIDTH && y >= 0 && y < SSD1306_HEIGHT) {
    if (on) {
      ssd1306_buffer[x + (y / SSD1306_FONT_WIDTH) * SSD1306_WIDTH] |= (1 << (y % SSD1306_FONT_WIDTH));
    } else {
      ssd1306_buffer[x + (y / SSD1306_FONT_WIDTH) * SSD1306_WIDTH] &= ~(1 << (y % SSD1306_FONT_WIDTH));
    }
  }
}

void ssd1306_draw_circle_points(int cx, int cy, int x, int y) {
    ssd1306_set_pixel(cx + x, cy + y, true);
    ssd1306_set_pixel(cx - x, cy + y, true);
    ssd1306_set_pixel(cx + x, cy - y, true);
    ssd1306_set_pixel(cx - x, cy - y, true);
    ssd1306_set_pixel(cx + y, cy + x, true);
    ssd1306_set_pixel(cx - y, cy + x, true);
    ssd1306_set_pixel(cx + y, cy - x, true);
    ssd1306_set_pixel(cx - y, cy - x, true);
}

void ssd1306_draw_circle(int cx, int cy, int radius, bool filled) {
    if (filled) {
        // Círculo preenchido
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x * x + y * y <= radius * radius) {
                    ssd1306_set_pixel(cx + x, cy + y, true);
                }
            }
        }
    } else {
        // Círculo vazio (apenas contorno)
        int x = 0;
        int y = radius;
        int d = 3 - 2 * radius;
        
        // Desenhar os 8 pontos simétricos iniciais
        ssd1306_draw_circle_points(cx, cy, x, y);
        
        while (y >= x) {
            x++;
            
            if (d > 0) {
                y--;
                d = d + 4 * (x - y) + 10;
            } else {
                d = d + 4 * x + 6;
            }
            
            ssd1306_draw_circle_points(cx, cy, x, y);
        }
    }
}

void ssd1306_draw_char(int x, int y, char c) {
  if (c < 32 || c > 126) c = 32; // Espaço para caracteres inválidos
  int index = c;

  for (int i = 0; i < SSD1306_FONT_WIDTH; i++) {
    for (int j = 0; j < SSD1306_FONT_WIDTH; j++) {
      bool pixel = (font8x8_basic[index][i] >> j) & 1;
      ssd1306_set_pixel(x + j, y + i, pixel);
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}

void ssd1306_draw_string(int x, int y, const char* str) {
  ESP_LOGI(TAG, "Desenhando string na posição (%d,%d): \"%s\"", x, y, str);
  while (*str) {
    ssd1306_draw_char(x, y, *str);
    x += SSD1306_FONT_WIDTH;
    str++;
  }
}

void ssd1306_draw_line(int x0, int y0, int x1, int y1) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    ssd1306_set_pixel(x0, y0, true);

    if (x0 == x1 && y0 == y1) break;

    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void ssd1306_draw_rect(int x, int y, int w, int h, bool filled) {
  if (filled) {
    for (int i = x; i < x + w; i++) {
      for (int j = y; j < y + h; j++) {
        ssd1306_set_pixel(i, j, true);
      }
    }
  } else {
    ssd1306_draw_line(x, y, x + w - 1, y);
    ssd1306_draw_line(x + w - 1, y, x + w - 1, y + h - 1);
    ssd1306_draw_line(x + w - 1, y + h - 1, x, y + h - 1);
    ssd1306_draw_line(x, y + h - 1, x, y);
  }
}