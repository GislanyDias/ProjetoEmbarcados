#ifndef PONG_H
#define PONG_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "ssd1306.h"    // Adicionar dependência do SSD1306
#include <stdio.h>      // Para snprintf

typedef struct {
    float x;
    float y;
    float dx;
    float dy;
} Ball;

typedef struct {
    int x;
    int width;
} Paddle;

void start_paddle_pong_game(void);
void init_pong(Ball *ball, Paddle *paddle);
void draw_ball(Ball *ball);
void draw_paddle(Paddle *paddle);

// Declarações de funções externas que são usadas
extern float accel_offset_x;  // Definida em dodge.c
extern void show_calibration_screen(void);  // Definida em dodge.c

#endif