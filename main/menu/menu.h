#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "mpu6050.h"
// Removi os includes dos jogos por enquanto - adicione quando implementar
// #include "tilt_maze.h"
// #include "paddle_pong.h"
// #include "dodge.h"
// #include "snake.h"

// GPIO definitions for buttons (corrigido conforme Wokwi)
#define BUTTON_UP_GPIO       GPIO_NUM_40   // Verde - navegação
#define BUTTON_SELECT_GPIO   GPIO_NUM_38   // Vermelho - seleção

// Display dimensions (assuming standard SSD1306)
#define DISPLAY_WIDTH        128
#define DISPLAY_HEIGHT       64

// Menu configuration
#define MAX_MENU_ITEMS       4
#define MENU_ITEM_HEIGHT     10
#define MENU_START_Y         16
#define MENU_ITEM_SPACING    2
#define TITLE_HEIGHT         12
#define INSTRUCTION_HEIGHT   8

// Menu layout definitions
#define SELECTION_ARROW_X    2
#define MENU_TEXT_X          12
#define SELECTION_BOX_MARGIN 1
#define TITLE_CENTER_X       32
#define INSTRUCTIONS_Y       (DISPLAY_HEIGHT - INSTRUCTION_HEIGHT)

// Button debounce time in milliseconds
#define BUTTON_DEBOUNCE_MS   50

// Button event types
typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED
} button_event_t;

// Button event data structure
typedef struct {
    gpio_num_t gpio_num;
    button_event_t event;
    TickType_t timestamp;
} button_event_data_t;

// Game selection enumeration
typedef enum {
    GAME_TILT_MAZE = 0,
    GAME_DODGE_BLOCKS,
    GAME_SNAKE_TILT,
    GAME_PADDLE_PONG
} game_selection_t;

// Menu state structure
typedef struct {
    int current_selection;
    bool menu_active;
    bool in_game;
} menu_state_t;

// Button state for debouncing
typedef struct {
    bool last_state;
    TickType_t last_change_time;
} button_state_t;

// === FUNCTION PROTOTYPES ===

// ISR Handler
void IRAM_ATTR button_isr_handler(void* arg);

// Menu drawing functions
void draw_title(void);
void draw_menu_item(int item_index, bool is_selected);
void draw_instructions(void);
void draw_status_info(void);
void draw_complete_menu(void);

// Game launch functions
void show_game_loading(game_selection_t game);
void launch_game(game_selection_t selected_game);
void return_to_menu(void);

// Button handling
bool is_button_pressed_debounced(gpio_num_t gpio_num);
void handle_menu_navigation(void);
void handle_menu_selection(void);
void handle_button_event(button_event_data_t* event_data);

// Button polling task
void button_polling_task(void* pvParameters);

// Initialization functions
esp_err_t init_menu_buttons(void);
void show_boot_screen(void);

// Main menu task
void menu_task(void* pvParameters);

// Game stubs (implementar depois)
void start_tilt_maze_game(void);
void start_dodge_blocks_game(void);
void start_snake_game(void);
void start_paddle_pong_game(void);

#endif