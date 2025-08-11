#include "tilt_maze.h"

static const uint8_t maze[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

extern float accel_offset_x;
extern float accel_offset_y;

void draw_maze() {
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            if (maze[y][x] == 1) {
                ssd1306_draw_rect(MAZE_OFFSET_X + x * CELL_SIZE, 
                                 MAZE_OFFSET_Y + y * CELL_SIZE, 
                                 CELL_SIZE, CELL_SIZE, true);
            }
        }
    }
    ssd1306_draw_rect(MAZE_OFFSET_X + MAZE_END_X * CELL_SIZE + 2, 
                     MAZE_OFFSET_Y + MAZE_END_Y * CELL_SIZE + 2, 
                     CELL_SIZE - 4, CELL_SIZE - 4, false);
}

void draw_maze_player(MazePlayer *player) {
    int px = MAZE_OFFSET_X + player->x * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2;
    int py = MAZE_OFFSET_Y + player->y * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2;
    ssd1306_draw_rect(px, py, PLAYER_SIZE, PLAYER_SIZE, true);
}

bool is_valid_move(int x, int y) {
    if (x < 0 || x >= MAZE_WIDTH || y < 0 || y >= MAZE_HEIGHT) {
        return false;
    }
    return maze[y][x] == 0;
}

void start_tilt_maze_game(void) {
    play_level_up();
    
    // Mostrar tela de calibração
    ssd1306_clear_buffer();
    ssd1306_draw_string(15, 10, "CALIBRANDO...");
    ssd1306_draw_string(10, 25, "MANTENHA PARADO");
    ssd1306_draw_string(25, 40, "3 SEGUNDOS");
    ssd1306_update_display();
    
    // Calibração do MPU6050
    mpu6050_data_t data;
    float sum_x = 0, sum_y = 0;
    int samples = 100;
    
    for (int i = 0; i < samples; i++) {
        if (mpu6050_read_all(&data) == ESP_OK) {
            sum_x += (float)data.accel_x / 16384.0f;
            sum_y += (float)data.accel_y / 16384.0f;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
    accel_offset_x = sum_x / samples;
    accel_offset_y = sum_y / samples;
    
    ssd1306_clear_buffer();
    ssd1306_draw_string(20, 20, "CALIBRADO!");
    ssd1306_draw_string(10, 35, "INCLINE PARA");
    ssd1306_draw_string(15, 50, "CONTROLAR");
    ssd1306_update_display();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    
    MazePlayer player = {1, 1};
    bool game_completed = false;
    uint32_t start_time = xTaskGetTickCount();
    button_event_data_t btn_event;
    
    while (1) {
        if (game_completed) {
            play_level_up();
            uint32_t time_taken = (xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS / 1000;
            
            ssd1306_clear_buffer();
            ssd1306_draw_string(15, 20, "PARABENS!");
            char time_text[30];
            snprintf(time_text, sizeof(time_text), "TEMPO: %lu", (unsigned long)time_taken);
            ssd1306_draw_string(15, 35, time_text);
            ssd1306_draw_string(5, 50, "PRESS ANY BUTTON");
            ssd1306_update_display();
            
            while (1) {
                if (button_get_event(&btn_event) == ESP_OK) {
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    return;
                }
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }
        }
        
        uint8_t accel_data[6];
        if (mpu6050_read_bytes(0x3B, accel_data, 6) == ESP_OK) {
            int16_t accel_x_raw = (int16_t)((accel_data[0] << 8) | accel_data[1]);
            int16_t accel_y_raw = (int16_t)((accel_data[2] << 8) | accel_data[3]);
            
            float accel_x = ((float)accel_x_raw / 16384.0f) - accel_offset_x;
            float accel_y = ((float)accel_y_raw / 16384.0f) - accel_offset_y;
            
            int new_x = player.x;
            int new_y = player.y;
            
            if (fabs(accel_x) > fabs(accel_y)) {
                if (accel_x > 0.2) new_x++;
                else if (accel_x < -0.2) new_x--;
            } else {
                if (accel_y > 0.2) new_y++;
                else if (accel_y < -0.2) new_y--;
            }
            
            if ((new_x != player.x || new_y != player.y) && is_valid_move(new_x, new_y)) {
                play_menu_navigate();
                player.x = new_x;
                player.y = new_y;
            }
            
            if (player.x == MAZE_END_X && player.y == MAZE_END_Y) {
                game_completed = true;
            }
        }
        
        ssd1306_clear_buffer();
        draw_maze();
        draw_maze_player(&player);
        
        uint32_t current_time = (xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS / 1000;
        char time_text[10];
        snprintf(time_text, sizeof(time_text), "%lu", (unsigned long)current_time);
        ssd1306_draw_string(98, 0, time_text);
        
        ssd1306_update_display();
        vTaskDelay(150 / portTICK_PERIOD_MS);
    }
}