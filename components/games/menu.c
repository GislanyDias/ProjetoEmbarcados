#include "menu.h"
#include "ssd1306.h"
#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "MENU";

// Configuração dos botões
#define BUTTON_1_GPIO 40  // Botão verde (navegação)
#define BUTTON_2_GPIO 38  // Botão vermelho (seleção)

// Variáveis de estado do menu
static MenuOption selected_option = MENU_OPTION_DODGE;
static bool option_changed = false;
static uint32_t last_button_press = 0;
static const uint32_t debounce_delay = 200; // 200ms

void menu_init(void) {
    // Configuração dos GPIOs para os botões
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_1_GPIO) | (1ULL << BUTTON_2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "MENU INICIADO");
}

void menu_update(MenuOption current_option) {
    // Atualiza o display com o menu
    ssd1306_clear_buffer();

    // Moldura principal
    ssd1306_draw_rect(2, 2, SSD1306_WIDTH-4, SSD1306_HEIGHT-4, false);

    // Linha decorativa
    ssd1306_draw_line(5, 15, SSD1306_WIDTH-6, 15);

    // Cabeçalho
    ssd1306_draw_string(SSD1306_WIDTH/2 - 20, 5, "= JOGOS =");

    // Opções do menu
    ssd1306_draw_string(20, 20, current_option == MENU_OPTION_DODGE ? "> DODGE" : "  DODGE");
    ssd1306_draw_string(20, 30, current_option == MENU_OPTION_TILT_MAZE ? "> TILT MAZE" : "  TILT MAZE");
    ssd1306_draw_string(20, 40, current_option == MENU_OPTION_SNAKE_TILT ? "> SNAKE TILT" : "  SNAKE TILT");
    ssd1306_draw_string(20, 50, current_option == MENU_OPTION_PADDLE_PONG ? "> PADDLE PONG" : "  PADDLE PONG");

    ssd1306_display_buffer();

    // Verifica entrada do usuário
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    if (now - last_button_press > debounce_delay) {
        int btn1_state = gpio_get_level(BUTTON_1_GPIO);
        int btn2_state = gpio_get_level(BUTTON_2_GPIO);
        
        // Botão 1 (Verde) - Navegação
        if (btn1_state == 0) {
            menu_play_nav_sound();
            last_button_press = now;
            selected_option = (selected_option + 1) % MENU_OPTION_COUNT;
            ESP_LOGI(TAG, "NAVEGANDO PARA: %d", selected_option);
        }
        
        // Botão 2 (Vermelho) - Seleção
        if (btn2_state == 0) {
            menu_play_select_sound();
            last_button_press = now;
            option_changed = true;
            ESP_LOGI(TAG, "OPCAO SELECIONADA: %d", selected_option);
        }
    }
    
    vTaskDelay(50 / portTICK_PERIOD_MS);
}

MenuOption menu_get_selected_option(void) {
    option_changed = false;
    return selected_option;
}

bool menu_option_selected(void) {
    return option_changed;
}

void menu_play_nav_sound(void) {
    play_tone(900, 50);
}

void menu_play_select_sound(void) {
    play_tone(1200, 80);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    play_tone(1500, 100);
}