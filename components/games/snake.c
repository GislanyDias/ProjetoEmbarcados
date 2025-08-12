#include "snake.h"

extern float accel_offset_x;
extern float accel_offset_y;

static float filtered_accel_x = 0.0f;
static float filtered_accel_y = 0.0f;

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

void show_snake_calibration_screen(void) {
    ssd1306_clear_buffer();
    ssd1306_draw_string(15, 10, "CALIBRANDO...");
    ssd1306_draw_string(5, 25, "DEIXE A PLACA");
    ssd1306_draw_string(10, 35, "COMPLETAMENTE");
    ssd1306_draw_string(25, 45, "PARADA");
    ssd1306_update_display();
    
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    
    for (int i = 5; i > 0; i--) {
        ssd1306_clear_buffer();
        char countdown[20];
        snprintf(countdown, sizeof(countdown), "AGUARDE: %d", i);
        ssd1306_draw_string(20, 30, countdown);
        ssd1306_update_display();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    ssd1306_clear_buffer();
    ssd1306_draw_string(10, 30, "CALIBRANDO...");
    ssd1306_update_display();
    
    mpu6050_data_t data;
    double sum_accel_x = 0, sum_accel_y = 0;
    int valid_samples = 0;
    int total_attempts = 500;
    
    for (int i = 0; i < total_attempts; i++) {
        if (mpu6050_read_all(&data) == ESP_OK) {
            float accel_x = (float)data.accel_x / 16384.0f;
            float accel_y = (float)data.accel_y / 16384.0f;
            
            if (fabs(accel_x) < 3.0f && fabs(accel_y) < 3.0f) {
                sum_accel_x += accel_x;
                sum_accel_y += accel_y;
                valid_samples++;
            }
        }
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
    
    if (valid_samples < 100) {
        accel_offset_x = 0.0f;
        accel_offset_y = 0.0f;
    } else {
        accel_offset_x = sum_accel_x / valid_samples;
        accel_offset_y = sum_accel_y / valid_samples;
    }
    
    ssd1306_clear_buffer();
    if (valid_samples >= 100) {
        ssd1306_draw_string(15, 15, "CALIBRADO!");
        ssd1306_draw_string(10, 30, "INCLINE PARA");
        ssd1306_draw_string(15, 45, "CONTROLAR");
        play_tone(1200, 200);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        play_tone(1500, 300);
    } else {
        ssd1306_draw_string(5, 20, "CALIBRACAO");
        ssd1306_draw_string(20, 35, "FALHOU!");
        play_tone(500, 500);
    }
    ssd1306_update_display();
    
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

void read_snake_sensor_data(float *accel_x_out, float *accel_y_out) {
    mpu6050_data_t data;
    static uint32_t last_read_time = 0;
    uint32_t current_time = xTaskGetTickCount();
    
    if (current_time - last_read_time < pdMS_TO_TICKS(50)) {
        *accel_x_out = filtered_accel_x;
        *accel_y_out = filtered_accel_y;
        return;
    }
    last_read_time = current_time;

    if (mpu6050_read_all(&data) != ESP_OK) {
        *accel_x_out = filtered_accel_x;
        *accel_y_out = filtered_accel_y;
        return;
    }

    float raw_accel_x = ((float)data.accel_x / 16384.0f) - accel_offset_x;
    float raw_accel_y = ((float)data.accel_y / 16384.0f) - accel_offset_y;

    const float alpha = 0.3f;
    filtered_accel_x = filtered_accel_x * (1.0f - alpha) + raw_accel_x * alpha;
    filtered_accel_y = filtered_accel_y * (1.0f - alpha) + raw_accel_y * alpha;

    *accel_x_out = filtered_accel_x;
    *accel_y_out = filtered_accel_y;
}

void start_snake_tilt_game(void) {
    play_level_up();
    char score_text[20];
    
    show_snake_calibration_screen();
    
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
            snprintf(score_text, sizeof(score_text), "SCORE:%d", score);
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
        
        float accel_x, accel_y;
        read_snake_sensor_data(&accel_x, &accel_y);
        
        const float threshold = 0.25f;  
        
        if (fabs(accel_x) > threshold || fabs(accel_y) > threshold) {
            if (accel_x > threshold && snake.direction != 3) {
                snake.next_direction = 1; // Direita
            } else if (accel_x < -threshold && snake.direction != 1) {
                snake.next_direction = 3; // Esquerda
            }

            if (accel_y > threshold && snake.direction != 0) {
                snake.next_direction = 0; // Baixo
            } else if (accel_y < -threshold && snake.direction != 2) {
                snake.next_direction = 2; // Cima
            }
        }
        
        uint32_t current_time = xTaskGetTickCount();
        if (current_time - last_move_time > (game_speed / portTICK_PERIOD_MS)) {
            last_move_time = current_time;
            
            snake.direction = snake.next_direction;
            
            SnakeSegment new_head = snake.segments[0];
            
            switch (snake.direction) {
                case 0: new_head.y--; break; // Cima
                case 1: new_head.x++; break; // Direita
                case 2: new_head.y++; break; // Baixo
                case 3: new_head.x--; break; // Esquerda
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