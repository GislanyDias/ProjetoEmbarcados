#ifndef DODGE_H
#define DODGE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "button.h"
#include "mpu6050.h"
#include "ssd1306.h"
#include "buzzer.h"
#include <math.h>

#define MAX_BLOCKS 3
#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 8
#define PLAYER_Y 50
#define BLOCK_WIDTH 6
#define BLOCK_HEIGHT 6
#define BLOCK_SPEED_INITIAL 1.0f
#define GAME_DELAY_MS 80

typedef struct {
    int x;
    int y;
    float speed;
    bool active;
} Block;

extern float accel_offset_x;
extern float accel_offset_y;

void start_dodge_blocks_game(void);
void show_calibration_screen(void);
void control_player_with_gyro(void);
void reset_game(void);
void draw_player(void);
void draw_block(Block *b);
bool check_collision(Block *b);

#endif