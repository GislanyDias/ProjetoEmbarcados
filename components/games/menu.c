#include "menu.h"
#include "ssd1306.h"
#include "buzzer.h"
#include "button.h" 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "MENU";

#define BUTTON_1_GPIO 40  
#define BUTTON_2_GPIO 38 

static MenuOption selected_option = MENU_OPTION_DODGE;
static bool option_changed = false;
static uint32_t last_button_press = 0;
static const uint32_t debounce_delay = 200; 

void menu_init(void) {
    ESP_LOGI(TAG, "MENU INICIADO");
}

void menu_update(MenuOption current_option) {
    ssd1306_clear_buffer();

    ssd1306_draw_rect(2, 2, SSD1306_WIDTH-4, SSD1306_HEIGHT-4, false);
    ssd1306_draw_line(5, 15, SSD1306_WIDTH-6, 15);
    ssd1306_draw_string(SSD1306_WIDTH/2 - 20, 5, "= JOGOS =");

    ssd1306_draw_string(20, 20, current_option == MENU_OPTION_DODGE ? "> DODGE" : "  DODGE");
    ssd1306_draw_string(20, 30, current_option == MENU_OPTION_TILT_MAZE ? "> TILT MAZE" : "  TILT MAZE");
    ssd1306_draw_string(20, 40, current_option == MENU_OPTION_SNAKE_TILT ? "> SNAKE TILT" : "  SNAKE TILT");
    ssd1306_draw_string(20, 50, current_option == MENU_OPTION_PADDLE_PONG ? "> PADDLE PONG" : "  PADDLE_PONG");

    ssd1306_update_display(); 

    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    if (now - last_button_press > debounce_delay) {
        int btn1_state = button_read_debounced(BUTTON_1_GPIO);
        int btn2_state = button_read_debounced(BUTTON_2_GPIO);
        
        ESP_LOGI(TAG, "BTN1: %d, BTN2: %d", btn1_state, btn2_state);
        
        if (btn1_state == 0) {
            menu_play_nav_sound();
            last_button_press = now;
            selected_option = (selected_option + 1) % MENU_OPTION_COUNT;
            ESP_LOGI(TAG, "NAVEGANDO PARA: %d", selected_option);
        }
        
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