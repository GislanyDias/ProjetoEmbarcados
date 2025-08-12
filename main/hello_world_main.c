#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "button.h"
#include "i2clib.h"
#include "mpu6050.h"
#include "ssd1306.h"
#include "buzzer.h"
#include "menu.h"
#include "dodge.h"
#include "tilt_maze.h"
#include "snake.h"
#include "pong.h"

static const char *TAG = "MAIN";

#define BUTTON_1_GPIO 40  
#define BUTTON_2_GPIO 38  

void app_main(void) {
    ESP_LOGI(TAG, "INICIANDO SISTEMA DE JOGOS");

    ESP_ERROR_CHECK(i2c_init());
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    gpio_config_t btn_config = {
        .pin_bit_mask = (1ULL << BUTTON_1_GPIO) | (1ULL << BUTTON_2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(init_buttons(&btn_config));

    buzzer_init();
    ssd1306_init();
    menu_init();

    esp_err_t mpu_ret = mpu6050_init();
    if (mpu_ret != ESP_OK) {
        ESP_LOGE(TAG, "FALHA MPU6050! ERRO: %s", esp_err_to_name(mpu_ret));
        
        ssd1306_clear_buffer();
        ssd1306_draw_string(10, 20, "ERRO MPU6050!");
        ssd1306_draw_string(5, 35, "VERIFIQUE AS");
        ssd1306_draw_string(15, 50, "CONEXOES");
        ssd1306_update_display();
        
        while(1) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    srand(time(NULL));

    play_tone(800, 100);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    play_tone(1200, 150);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    play_tone(1600, 200);

    ESP_LOGI(TAG, "SISTEMA INICIADO");

    while (1) {
        MenuOption current_option = menu_get_selected_option();
        menu_update(current_option);

        if (menu_option_selected()) {
            current_option = menu_get_selected_option();

            switch(current_option) {
                case MENU_OPTION_DODGE:
                    ESP_LOGI(TAG, "INICIANDO DODGE");
                    start_dodge_blocks_game();
                    break;
                case MENU_OPTION_TILT_MAZE:
                    ESP_LOGI(TAG, "INICIANDO TILT MAZE");
                    start_tilt_maze_game();
                    break;
                case MENU_OPTION_SNAKE_TILT:
                    ESP_LOGI(TAG, "INICIANDO SNAKE TILT");
                    start_snake_tilt_game();
                    break;
                case MENU_OPTION_PADDLE_PONG:
                    ESP_LOGI(TAG, "INICIANDO PADDLE PONG");
                    start_paddle_pong_game();
                    break;
                default:
                    break;
            }

            ssd1306_clear_buffer();
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}