#include "dodge.h"
#include "display.h"
#include "mpu6050.h"
#include "buzzer.h"

static int player_x = 128 / 2;
static int score = 0;
static bool game_over = false;
static Block blocks[MAX_BLOCKS];
static float player_velocity = 0.0f;

void reset_game() {
    player_x = 128 / 2;
    score = 0;
    game_over = false;
    for (int i = 0; i < MAX_BLOCKS; i++) {
        blocks[i].x = rand() % (128 - BLOCK_WIDTH);
        blocks[i].y = -(rand() % 40);
        blocks[i].speed = BLOCK_SPEED_INITIAL + (rand() % 30) / 10.0f;
        blocks[i].active = true;
    }
}

void draw_player() {
    ssd1306_draw_rect(player_x, PLAYER_Y, PLAYER_WIDTH, PLAYER_HEIGHT, true);
}

void draw_block(Block *b) {
    if (b->active)
        ssd1306_draw_rect(b->x, b->y, BLOCK_WIDTH, BLOCK_HEIGHT, true);
}

bool check_collision(Block *b) {
    return (b->x < player_x + PLAYER_WIDTH && b->x + BLOCK_WIDTH > player_x &&
            b->y < PLAYER_Y + PLAYER_HEIGHT && b->y + BLOCK_HEIGHT > PLAYER_Y);
}

void control_player_with_gyro(void) {
    float gyro_x, gyro_y, gyro_z;
    uint8_t accel_data[6];
    
    bool gyro_ok = (mpu6050_read_gyro(&gyro_x, &gyro_y, &gyro_z) == ESP_OK);
    bool accel_ok = (mpu6050_read_bytes(0x3B, accel_data, 6) == ESP_OK);
    
    if (gyro_ok && accel_ok) {
        int16_t accel_x_raw = (int16_t)((accel_data[0] << 8) | accel_data[1]);
        int16_t accel_y_raw = (int16_t)((accel_data[2] << 8) | accel_data[3]);
        
        float accel_x = ((float)accel_x_raw / 16384.0f) - accel_offset_x;
        float accel_y = ((float)accel_y_raw / 16384.0f) - accel_offset_y;
        
        float tilt_angle = atan2(accel_x, accel_y) * 180.0f / M_PI;
        
        const float tilt_dead_zone = 5.0f;
        if (fabs(tilt_angle) < tilt_dead_zone) {
            tilt_angle = 0.0f;
        }
        
        const float gyro_dead_zone = 3.0f;
        if (fabs(gyro_x) < gyro_dead_zone) {
            gyro_x = 0.0f;
        }
        
        float tilt_movement = tilt_angle * 0.8f;
        float gyro_movement = gyro_x * 0.4f;
        float total_movement = tilt_movement + gyro_movement;
        
        static float filtered_movement = 0.0f;
        filtered_movement = filtered_movement * 0.6f + total_movement * 0.4f;
        
        static float target_velocity = 0.0f;
        target_velocity = filtered_movement * 0.15f;
        
        player_velocity = player_velocity * 0.8f + target_velocity * 0.2f;
        
        if (player_velocity > 8.0f) player_velocity = 8.0f;
        if (player_velocity < -8.0f) player_velocity = -8.0f;
        
        player_x += (int)player_velocity;
        
        if (player_x < 0) player_x = 0;
        if (player_x > 128 - PLAYER_WIDTH) player_x = 128 - PLAYER_WIDTH;
    }
}

void show_calibration_screen(void) {
    ssd1306_clear_buffer();
    ssd1306_draw_string(15, 10, "CALIBRANDO...");
    ssd1306_draw_string(10, 25, "MANTENHA PARADO");
    ssd1306_draw_string(25, 40, "3 SEGUNDOS");
    ssd1306_update_display();
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    ssd1306_clear_buffer();
    ssd1306_draw_string(50, 30, "2...");
    ssd1306_update_display();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    ssd1306_clear_buffer();
    ssd1306_draw_string(50, 30, "1...");
    ssd1306_update_display();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    mpu6050_calibrate_improved();
    
    ssd1306_clear_buffer();
    ssd1306_draw_string(20, 20, "CALIBRADO!");
    ssd1306_draw_string(10, 35, "INCLINE PARA");
    ssd1306_draw_string(15, 50, "CONTROLAR");
    ssd1306_update_display();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

void start_dodge_blocks_game(void) {
    play_level_up();
    show_calibration_screen();
    reset_game();
    
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

        control_player_with_gyro();

        for (int i = 0; i < MAX_BLOCKS; i++) {
            if (!blocks[i].active) continue;
            blocks[i].y += (int)blocks[i].speed;
            if (blocks[i].y > 64) {
                blocks[i].y = -(rand() % 30);
                blocks[i].x = rand() % (128 - BLOCK_WIDTH);
                blocks[i].speed += 0.1f;
                score++;
                play_point_scored();
            }
            if (check_collision(&blocks[i])){
                play_game_over();
                game_over = true;
            }
        }

        ssd1306_clear_buffer();
        draw_player();
        for (int i = 0; i < MAX_BLOCKS; i++)
            draw_block(&blocks[i]);
        
        char score_text[16];
        snprintf(score_text, sizeof(score_text), "%d", score);
        ssd1306_draw_string(98, 0, score_text);
        ssd1306_update_display();
        vTaskDelay(pdMS_TO_TICKS(GAME_DELAY_MS));
    }
}