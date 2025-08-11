#include "pong.h"
#include "ssd1306.h"  // CORREÇÃO: usar ssd1306.h em vez de display.h
#include "mpu6050.h"
#include "buzzer.h"
#include "dodge.h"     // Para usar accel_offset_x
#include <stdlib.h>    // Para rand()

void init_pong(Ball *ball, Paddle *paddle) {
    ball->x = 128 / 2;
    ball->y = 64 / 2;
    ball->dx = 1.5f;
    ball->dy = 1.5f;
    
    paddle->x = 128 / 2 - 15;
    paddle->width = 30;
}

void draw_ball(Ball *ball) {
    ssd1306_draw_rect((int)ball->x - 2, (int)ball->y - 2, 4, 4, true);
}

void draw_paddle(Paddle *paddle) {
    ssd1306_draw_rect(paddle->x, 64 - 5, paddle->width, 3, true);
}

void start_paddle_pong_game(void) {
    play_level_up();
    show_calibration_screen();  // Esta função está em dodge.c
    
    Ball ball;
    Paddle paddle;
    init_pong(&ball, &paddle);
    
    int score = 0;
    int lives = 3;
    bool game_over = false;
    
    while (1) {
        if (game_over) {
            ssd1306_clear_buffer();
            ssd1306_draw_rect(2, 2, 124, 60, false);
            ssd1306_draw_string(20, 20, "GAME OVER");
            char score_text[20];
            snprintf(score_text, sizeof(score_text), "ESCORE: %d", score);
            ssd1306_draw_string(20, 35, score_text);
            ssd1306_draw_string(5, 50, "PRESS ANY BUTTON");
            ssd1306_update_display();
            
            while (1) {
                if (gpio_get_level(40) == 0 || gpio_get_level(38) == 0) {
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    return;
                }
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }
        }
        
        // Controle do paddle usando MPU6050
        uint8_t accel_data[6];
        if (mpu6050_read_bytes(0x3B, accel_data, 6) == ESP_OK) {
            int16_t accel_x_raw = (int16_t)((accel_data[0] << 8) | accel_data[1]);
            float accel_x = ((float)accel_x_raw / 16384.0f) - accel_offset_x;
            
            paddle.x += (int)(accel_x * 5.0f);
            
            if (paddle.x < 0) paddle.x = 0;
            if (paddle.x > 128 - paddle.width) paddle.x = 128 - paddle.width;
        }
        
        // Movimento da bola
        ball.x += ball.dx;
        ball.y += ball.dy;
        
        // Colisão com paredes laterais
        if (ball.x <= 2 || ball.x >= 128 - 2) {
            ball.dx = -ball.dx;
        }
        
        // Colisão com parede superior
        if (ball.y <= 2) {
            ball.dy = -ball.dy;
        }
        
        // Colisão com paddle
        if (ball.y >= 64 - 7 && 
            ball.x >= paddle.x && ball.x <= paddle.x + paddle.width) {
            play_point_scored();
            ball.dy = -ball.dy;
            float hit_pos = (ball.x - (paddle.x + paddle.width / 2)) / (paddle.width / 2.0f);
            ball.dx = hit_pos * 2.0f;
            
            score++;
        }
        
        // Bola saiu pela parte inferior
        if (ball.y >= 64) {
            lives--;
            if (lives <= 0) {
                play_game_over();
                game_over = true;
            } else {
                // Resetar posição da bola
                ball.x = 128 / 2;
                ball.y = 64 / 2;
                ball.dx = (rand() % 2 == 0) ? 1.5f : -1.5f;
                ball.dy = 1.5f;
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }
        
        // Desenhar tudo
        ssd1306_clear_buffer();
        draw_ball(&ball);
        draw_paddle(&paddle);
        
        // Mostrar pontuação e vidas
        char score_text[20];
        snprintf(score_text, sizeof(score_text), "SCORE:%d", score);
        ssd1306_draw_string(5, 5, score_text);
        
        char lives_text[10];
        snprintf(lives_text, sizeof(lives_text), "VIDA:%d", lives);
        ssd1306_draw_string(78, 5, lives_text);
        
        ssd1306_update_display();
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}