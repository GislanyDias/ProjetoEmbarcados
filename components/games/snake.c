#include "snake.h"

extern float accel_offset_x;
extern float accel_offset_y;

void init_snake(Snake *snake) {
    snake->length = 3;
    snake->direction = 1;
    snake->next_direction = 1;
    
    for (int i = 0; i < snake->length; i++) {
        snake->segments[i].x = (128 / 8 / 2) - i;
        snake->segments[i].y = 64 / 8 / 2;
    }
}

void generate_food(Food *food, Snake *snake) {
    bool valid_position = false;
    int attempts = 0;
    const int max_attempts = 50;
    
    while (!valid_position && attempts < max_attempts) {
        attempts++;
        food->x = 1 + rand() % ((128 / 8) - 2);
        food->y = 1 + rand() % ((64 / 8) - 2);
        
        valid_position = true;
        for (int i = 0; i < snake->length; i++) {
            bool near_head = (i == 0) && 
                (abs(snake->segments[0].x - food->x) <= 2) && 
                (abs(snake->segments[0].y - food->y) <= 2);
                
            if ((snake->segments[i].x == food->x && 
                 snake->segments[i].y == food->y) ||
                near_head) {
                valid_position = false;
                break;
            }
        }
    }
    
    if (!valid_position) {
        food->x = 128 / 8 / 2;
        food->y = 64 / 8 / 2;
    }
}

void draw_snake(Snake *snake) {
    for (int i = 0; i < snake->length; i++) {
        if (i == 0) {
            ssd1306_draw_rect(snake->segments[i].x * 8, snake->segments[i].y * 8, 8, 8, true);
        } else {
            ssd1306_draw_rect(snake->segments[i].x * 8 + 1, snake->segments[i].y * 8 + 1, 6, 6, true);
        }
    }
}

void draw_food(Food *food) {
    int center_x = food->x * 8 + 4;
    int center_y = food->y * 8 + 4;
    int radius = 3;
    
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                ssd1306_set_pixel(center_x + x, center_y + y, true);
            }
        }
    }
}

bool check_collision_snake(Snake *snake) {
    if (snake->segments[0].x < 0 || snake->segments[0].x >= 128 / 8 ||
        snake->segments[0].y < 0 || snake->segments[0].y >= 64 / 8) {
        return true;
    }
    
    for (int i = 4; i < snake->length; i++) {
        if (snake->segments[0].x == snake->segments[i].x && 
            snake->segments[0].y == snake->segments[i].y) {
            return true;
        }
    }
    
    return false;
}

int map(int x, int in_min, int in_max, int out_min, int out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void start_snake_tilt_game(void) {
    play_level_up();
    char score_text[20];
    
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
    
    Snake snake;
    Food food;
    init_snake(&snake);
    generate_food(&food, &snake);
    
    int score = 0;
    int lives = MAX_LIVES;
    bool game_over = false;
    int game_speed = INITIAL_SNAKE_SPEED;
    uint32_t last_move_time = xTaskGetTickCount();
    button_event_data_t btn_event;

    while (1) {
        if (game_over) {
            ssd1306_clear_buffer();
            ssd1306_draw_rect(2, 2, 124, 60, false);
            ssd1306_draw_string(20, 20, "GAME OVER");
            snprintf(score_text, sizeof(score_text), "ESCORE: %d", score);
            ssd1306_draw_string(20, 35, score_text);
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
            
            const float threshold = 0.15;
            
            if (accel_x > threshold && snake.direction != 3) 
                snake.next_direction = 1;
            else if (accel_x < -threshold && snake.direction != 1) 
                snake.next_direction = 3;

            if (accel_y > threshold && snake.direction != 0) 
                snake.next_direction = 2;
            else if (accel_y < -threshold && snake.direction != 2) 
                snake.next_direction = 0;

            snake.direction = snake.next_direction;
        }
        
        uint32_t current_time = xTaskGetTickCount();
        if (current_time - last_move_time > (game_speed / portTICK_PERIOD_MS)) {
            last_move_time = current_time;
            
            snake.direction = snake.next_direction;
            
            SnakeSegment new_head = snake.segments[0];
            
            switch (snake.direction) {
                case 0: new_head.y--; break;
                case 1: new_head.x++; break;
                case 2: new_head.y++; break;
                case 3: new_head.x--; break;
            }
            
            if (check_collision_snake(&snake)) {
                lives--;
                
                if (lives <= 0) {
                    play_game_over();
                    game_over = true;
                    continue;
                } else {
                    init_snake(&snake);
                    game_speed = INITIAL_SNAKE_SPEED;
                    continue;
                }
            }
            
            for (int i = snake.length - 1; i > 0; i--) {
                snake.segments[i] = snake.segments[i - 1];
            }
            snake.segments[0] = new_head;
            
            if (new_head.x == food.x && new_head.y == food.y) {
                play_point_scored();
                if (snake.length < MAX_SNAKE_SEGMENTS) {
                    snake.segments[snake.length] = snake.segments[snake.length - 1];
                    snake.length++;
                    score += 10;
                    
                    if (game_speed > MIN_SNAKE_SPEED) {
                        game_speed -= SPEED_DECREMENT;
                    }
                    
                    for (int blink = 0; blink < 3; blink++) {
                        ssd1306_clear_buffer();
                        ssd1306_draw_string(128/2 - 20, 64/2, "+10");
                        ssd1306_update_display();
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        
                        ssd1306_clear_buffer();
                        draw_snake(&snake);
                        draw_food(&food);
                        ssd1306_draw_string(98, 0, score_text);
                        ssd1306_update_display();
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                    }
                }
                
                generate_food(&food, &snake);
            }
        }
        
        ssd1306_clear_buffer();
        draw_snake(&snake);
        draw_food(&food);
        
        snprintf(score_text, sizeof(score_text), "SCORE: %d", score);
        ssd1306_draw_string(5, 0, score_text);
        
        char lives_text[15];
        snprintf(lives_text, sizeof(lives_text), "LIVES: %d", lives);
        ssd1306_draw_string(68, 0, lives_text);
        
        int speed_indicator = map(game_speed, MIN_SNAKE_SPEED, INITIAL_SNAKE_SPEED, 5, 123);
        ssd1306_draw_rect(5, 61, speed_indicator, 2, true);
        
        ssd1306_update_display();
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}