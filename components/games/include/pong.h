#ifndef PONG_H
#define PONG_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

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

#endif