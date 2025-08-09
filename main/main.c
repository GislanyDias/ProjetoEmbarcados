#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "mpu6050.h"
#include "ssd1306.h"
#include "buzzer.h"
#include "menu.h"
#include "dodge.h"
#include "tilt_maze.h"
#include "snake.h"
#include "pong.h"

static const char *TAG = "MAIN";

// Configurações I2C
#define I2C_MASTER_SCL_IO           9
#define I2C_MASTER_SDA_IO           8
#define I2C_MASTER_NUM              0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_TIMEOUT_MS       1000

// Inicialização do I2C
esp_err_t i2c_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 
                             I2C_MASTER_RX_BUF_DISABLE, 
                             I2C_MASTER_TX_BUF_DISABLE, 0);
}

void app_main(void) {
    ESP_LOGI(TAG, "INICIANDO SISTEMA DE JOGOS");

    // Inicializações básicas
    ESP_ERROR_CHECK(i2c_init());
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    // Inicializa componentes
    buzzer_init();
    ssd1306_init();
    menu_init();

    // Inicializa MPU6050 (giroscópio/acelerômetro)
    esp_err_t mpu_ret = mpu6050_init();
    if (mpu_ret != ESP_OK) {
        ESP_LOGE(TAG, "FALHA MPU6050! ERRO: %s", esp_err_to_name(mpu_ret));
        
        // Mostra mensagem de erro no display
        ssd1306_clear_buffer();
        ssd1306_draw_string(10, 20, "ERRO MPU6050!");
        ssd1306_draw_string(5, 35, "VERIFIQUE AS");
        ssd1306_draw_string(15, 50, "CONEXOES");
        ssd1306_update_display();
        
        while(1) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    // Inicializa gerador de números aleatórios
    srand(time(NULL));

    // Toca som de inicialização
    play_tone(800, 100);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    play_tone(1200, 150);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    play_tone(1600, 200);

    ESP_LOGI(TAG, "SISTEMA INICIADO");

    // Variável para armazenar a opção atual do menu
    MenuOption current_option = MENU_OPTION_DODGE;

    // Loop principal
    while (1) {
        // Atualiza o menu
        menu_update(current_option);

        // Verifica se uma opção foi selecionada
        if (menu_option_selected()) {
            current_option = menu_get_selected_option();

            // Executa o jogo selecionado
            switch(current_option) {
                case MENU_OPTION_DODGE:
                    ESP_LOGI(TAG, "INICIANDO DODGE");
                    start_dodge_game();
                    break;
                case MENU_OPTION_TILT_MAZE:
                    ESP_LOGI(TAG, "INICIANDO TILT MAZE");
                    start_tilt_maze_game();
                    break;
                case MENU_OPTION_SNAKE_TILT:
                    ESP_LOGI(TAG, "INICIANDO SNAKE TILT");
                    start_snake_game();
                    break;
                case MENU_OPTION_PADDLE_PONG:
                    ESP_LOGI(TAG, "INICIANDO PADDLE PONG");
                    start_pong_game();
                    break;
                default:
                    break;
            }

            // Após sair do jogo, volta para o menu
            ssd1306_clear_buffer();
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}