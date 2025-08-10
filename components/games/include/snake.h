#ifndef SNAKE_TILT_H
#define SNAKE_TILT_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define MAX_SNAKE_SEGMENTS 50
#define INITIAL_SNAKE_SPEED 200
#define MIN_SNAKE_SPEED 80
#define SPEED_DECREMENT 10
#define MAX_LIVES 3

typedef struct {
    int x;
    int y;
} SnakeSegment;

typedef struct {
    SnakeSegment segments[MAX_SNAKE_SEGMENTS];
    int length;
    int direction;
    int next_direction;
} Snake;

typedef struct {
    int x;
    int y;
} Food;

void start_snake_tilt_game(void);
void init_snake(Snake *snake);
void generate_food(Food *food, Snake *snake);
void draw_snake(Snake *snake);
void draw_food(Food *food);
bool check_collision_snake(Snake *snake);

#endif