#include "games/games.h"
#include "games/dodge.h"
#include "games/tilt_maze.h"
#include "games/snake.h"
#include "games/pong.h"
#include "ssd1306/ssd1306.h"
#include "mpu6050/mpu6050.h"
#include "buzzer/buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "GAMES";

// Variáveis do menu
static MenuOption current_option = MENU_OPTION_DODGE;
static bool display_updated = false;

// Configuração dos botões
#define BUTTON_1_GPIO 40  // Botão verde
#define BUTTON_2_GPIO 38  // Botão vermelho

void games_init(void) {
    ESP_LOGI(TAG, "Inicializando sistema de jogos");
    
    // Inicializa os botões
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_1_GPIO) | (1ULL << BUTTON_2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "Sistema de jogos inicializado");
}

static void draw_menu(void) {
    ssd1306_clear_buffer();

    // Moldura principal
    ssd1306_draw_rect(2, 2, SSD1306_WIDTH-4, SSD1306_HEIGHT-4, false);

    // Cabeçalho
    ssd1306_draw_string(SSD1306_WIDTH/2 - 20, 5, "JOGOS");

    // Opções do menu
    ssd1306_draw_string(20, 20, current_option == MENU_OPTION_DODGE ? "> DODGE" : "  DODGE");
    ssd1306_draw_string(20, 30, current_option == MENU_OPTION_TILT_MAZE ? "> TILT MAZE" : "  TILT MAZE");
    ssd1306_draw_string(20, 40, current_option == MENU_OPTION_SNAKE_TILT ? "> SNAKE TILT" : "  SNAKE TILT");
    ssd1306_draw_string(20, 50, current_option == MENU_OPTION_PADDLE_PONG ? "> PADDLE PONG" : "  PADDLE PONG");

    ssd1306_update_display();
    display_updated = true;
}

void games_run(void) {
    uint32_t last_button_press = 0;
    const uint32_t debounce_delay = 200; // 200ms
    
    while (1) {
        if (!display_updated) {
            draw_menu();
        }
        
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        if (now - last_button_press > debounce_delay) {
            int btn1_state = gpio_get_level(BUTTON_1_GPIO);
            int btn2_state = gpio_get_level(BUTTON_2_GPIO);
            
            // Botão 1 (Verde) - Navegação
            if (btn1_state == 0) {
                play_menu_navigate();
                last_button_press = now;
                current_option = (current_option + 1) % MENU_OPTION_COUNT;
                ESP_LOGI(TAG, "Opção do menu: %d", current_option);
                display_updated = false;
            }
            
            // Botão 2 (Vermelho) - Seleção
            if (btn2_state == 0) {
                play_menu_select();
                last_button_press = now;
                ESP_LOGI(TAG, "Jogo selecionado: %d", current_option);
                
                switch(current_option) {
                    case MENU_OPTION_DODGE:
                        start_dodge_blocks_game();
                        display_updated = false;
                        break;
                    case MENU_OPTION_TILT_MAZE:
                        start_tilt_maze_game();
                        display_updated = false;
                        break;
                    case MENU_OPTION_SNAKE_TILT:
                        start_snake_tilt_game();
                        display_updated = false;
                        break;
                    case MENU_OPTION_PADDLE_PONG:
                        start_paddle_pong_game();
                        display_updated = false;
                        break;
                    default:
                        break;
                }
            }
        }
        
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}