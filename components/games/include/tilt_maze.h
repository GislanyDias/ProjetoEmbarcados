#ifndef TILT_MAZE_H
#define TILT_MAZE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "button.h"
#include "mpu6050.h"
#include "ssd1306.h"
#include <math.h>
#include "buzzer.h"

#define MAZE_WIDTH 16
#define MAZE_HEIGHT 8
#define CELL_SIZE 8
#define PLAYER_SIZE 4
#define MAZE_OFFSET_X ((128 - MAZE_WIDTH * CELL_SIZE) / 2)
#define MAZE_OFFSET_Y ((64 - MAZE_HEIGHT * CELL_SIZE) / 2)
#define MAZE_END_X 14
#define MAZE_END_Y 6

extern float accel_offset_x;
extern float accel_offset_y;

typedef struct {
    int x;
    int y;
} MazePlayer;

void start_tilt_maze_game(void);
void draw_maze(void);
void draw_maze_player(MazePlayer *player);
bool is_valid_move(int x, int y);

#endif