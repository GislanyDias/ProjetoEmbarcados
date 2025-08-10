#include "buzzer.h"
#include "esp_log.h"

void buzzer_init() {
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    ledc_channel_config_t channel_conf = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    
    ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));
    
    // Garante que comece desligado
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void play_tone(int frequency, int duration_ms) {
    if (frequency <= 150) {
        frequency = 150;
    }
    
    // Configura a frequência
    esp_err_t err = ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, frequency);
    if (err != ESP_OK) {
        ESP_LOGE(BUZZER_TAG, "Falha ao definir frequência %dHz: %s", 
                frequency, esp_err_to_name(err));
        return;
    }

    // Duty cycle de 50% (128 em 255)
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 128);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    
    // Desliga o buzzer
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void play_game_over() {
    play_tone(1000, 200);
    vTaskDelay(pdMS_TO_TICKS(50));
    play_tone(800, 150);
    vTaskDelay(pdMS_TO_TICKS(50));
    play_tone(600, 250);
    play_tone(400, 350);
}

void play_menu_select() {
    play_tone(1200, 80);
    vTaskDelay(pdMS_TO_TICKS(20));
    play_tone(1500, 100);
}

void play_menu_navigate() {
    play_tone(900, 50);
}

void play_point_scored() {
    play_tone(1300, 80);
    vTaskDelay(pdMS_TO_TICKS(30));
    play_tone(1600, 60);
}

void play_level_up() {
    for (int i = 0; i < 3; i++) {
        play_tone(1000 + (i * 300), 50);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}