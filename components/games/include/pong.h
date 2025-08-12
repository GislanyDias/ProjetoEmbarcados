#ifndef PONG_H
#define PONG_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "ssd1306.h"
#include <stdio.h>

#define PADDLE_WIDTH 30
#define PADDLE_HEIGHT 3
#define PADDLE_Y (64 - 5)
#define BALL_SIZE 4
#define INITIAL_LIVES 3

typedef struct {
    float x;
    float y;
    float dx;
    float dy;
} Ball;

typedef struct {
    int x;
    int width;
    int speed;
} Paddle;

void start_paddle_pong_game(void);
void pong_init_game(Ball *ball, Paddle *paddle);
void pong_draw_ball(Ball *ball);
void pong_draw_paddle(Paddle *paddle);
void pong_control_paddle(Paddle *paddle);
void pong_show_game_over(int score);

#endif