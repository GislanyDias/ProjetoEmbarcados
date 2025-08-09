#include "menu.h"

// Global menu state
static menu_state_t menu_state = {0, true, false};

// Button states for debouncing
static button_state_t button_states[2] = {0};

// Static menu items array
static const char* menu_items[MAX_MENU_ITEMS] = {
    "tilt maze",
    "dodge the blocks", 
    "snake tilt",
    "paddle pong"
};

// Static menu descriptions (optional for status bar)
static const char* menu_descriptions[MAX_MENU_ITEMS] = {
    "navigate maze by tilting",
    "avoid falling blocks",
    "snake controlled by tilt",
    "classic pong with tilt"
};

// Button event queue
static QueueHandle_t button_queue = NULL;

// === BUTTON INTERRUPT HANDLER ===

void IRAM_ATTR button_isr_handler(void* arg)
{
    gpio_num_t gpio_num = (gpio_num_t)(uint32_t)arg;
    
    button_event_data_t event_data = {
        .gpio_num = gpio_num,
        .event = BUTTON_EVENT_PRESSED,
        .timestamp = xTaskGetTickCountFromISR()
    };

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(button_queue, &event_data, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// === MENU DRAWING FUNCTIONS ===

void draw_title(void)
{
    // Draw main title with border
    ssd1306_draw_string(TITLE_CENTER_X, 0, "game menu");
    
    // Draw underline for title
    ssd1306_draw_line(0, TITLE_HEIGHT - 2, DISPLAY_WIDTH - 1, TITLE_HEIGHT - 2);
}

void draw_menu_item(int item_index, bool is_selected)
{
    int y_pos = MENU_START_Y + (item_index * (MENU_ITEM_HEIGHT + MENU_ITEM_SPACING));
    
    if (is_selected) {
        // Draw selection background box (filled rectangle)
        ssd1306_draw_rect(0, y_pos - SELECTION_BOX_MARGIN, 
                         DISPLAY_WIDTH, MENU_ITEM_HEIGHT + (2 * SELECTION_BOX_MARGIN), 
                         false); // false = filled rectangle
        
        // Draw selection arrow
        ssd1306_draw_string(SELECTION_ARROW_X, y_pos, ">");
        
        // Draw item text normally (will appear inverted due to filled background)
        ssd1306_draw_string(MENU_TEXT_X, y_pos, menu_items[item_index]);
    } else {
        // Draw normal item text
        ssd1306_draw_string(MENU_TEXT_X, y_pos, menu_items[item_index]);
    }
}

void draw_instructions(void)
{
    // Draw control instructions at bottom
    ssd1306_draw_string(0, INSTRUCTIONS_Y, "up:navigate  select:play");
}

void draw_status_info(void)
{
    // Optional: Draw current selection info
    char status_text[32];
    snprintf(status_text, sizeof(status_text), "%d/%d", 
             menu_state.current_selection + 1, MAX_MENU_ITEMS);
    ssd1306_draw_string(DISPLAY_WIDTH - 24, 0, status_text);
}

void draw_complete_menu(void)
{
    // Clear the display buffer
    ssd1306_clear_buffer();
    
    // Draw all menu components
    draw_title();
    
    // Draw all menu items
    for (int i = 0; i < MAX_MENU_ITEMS; i++) {
        draw_menu_item(i, (i == menu_state.current_selection));
    }
    
    draw_instructions();
    draw_status_info();
    
    // Update the physical display
    ssd1306_update_display();
}

// === GAME LAUNCH FUNCTIONS ===

void show_game_loading(game_selection_t game)
{
    ssd1306_clear_buffer();
    
    // Show loading screen
    ssd1306_draw_string(25, 15, "loading...");
    ssd1306_draw_string(10, 30, menu_items[game]);
    
    // Draw simple loading animation
    ssd1306_draw_rect(20, 45, 88, 8, true); // Loading bar border
    ssd1306_draw_rect(22, 47, 84, 4, false); // Loading bar fill
    
    ssd1306_update_display();
    vTaskDelay(1500 / portTICK_PERIOD_MS); // Simulate loading
}

void launch_game(game_selection_t selected_game)
{
    ESP_LOGI("menu", "launching game: %s", menu_items[selected_game]);
    
    show_game_loading(selected_game);
    
    // Show game placeholder screen
    ssd1306_clear_buffer();
    ssd1306_draw_string(0, 5, "now playing:");
    ssd1306_draw_string(0, 20, menu_items[selected_game]);
    ssd1306_draw_string(0, 35, menu_descriptions[selected_game]);
    ssd1306_draw_line(0, 47, DISPLAY_WIDTH - 1, 47);
    ssd1306_draw_string(0, 50, "up: return to menu");
    ssd1306_update_display();
    
    menu_state.in_game = true;
    menu_state.menu_active = false;
    
    // TODO: Here you would call the actual game functions
    switch (selected_game) {
        case GAME_TILT_MAZE:
            start_tilt_maze_game();
            break;
        case GAME_DODGE_BLOCKS:
            start_dodge_blocks_game();
            break;
        case GAME_SNAKE_TILT:
            start_snake_game();
            break;
        case GAME_PADDLE_PONG:
            start_paddle_pong_game();
            break;
    }
}

void return_to_menu(void)
{
    ESP_LOGI("MENU", "Returning to main menu");
    
    menu_state.menu_active = true;
    menu_state.in_game = false;
    
    // Redraw the menu
    draw_complete_menu();
}

// === BUTTON HANDLING ===

bool is_button_pressed_debounced(gpio_num_t gpio_num)
{
    int button_index = (gpio_num == BUTTON_UP_GPIO) ? 0 : 1;
    bool current_state = !gpio_get_level(gpio_num); // Inverted because of pull-up
    TickType_t current_time = xTaskGetTickCount();
    
    // Check if state has changed
    if (current_state != button_states[button_index].last_state) {
        button_states[button_index].last_change_time = current_time;
        button_states[button_index].last_state = current_state;
        return false; // Don't register press immediately after state change
    }
    
    // Check if enough time has passed since last change (debouncing)
    if ((current_time - button_states[button_index].last_change_time) > pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS)) {
        return current_state; // Button state is stable
    }
    
    return false;
}

void handle_menu_navigation(void)
{
    // Move to previous item (wrapping around)
    menu_state.current_selection = (menu_state.current_selection - 1 + MAX_MENU_ITEMS) % MAX_MENU_ITEMS;
    ESP_LOGI("MENU", "Navigated to: %s", menu_items[menu_state.current_selection]);
    
    // Redraw menu with new selection
    draw_complete_menu();
}

void handle_menu_selection(void)
{
    game_selection_t selected_game = (game_selection_t)menu_state.current_selection;
    launch_game(selected_game);
}

void handle_button_event(button_event_data_t* event_data)
{
    if (event_data->event != BUTTON_EVENT_PRESSED) {
        return; // Only handle press events
    }
    
    // Add debouncing check
    if (!is_button_pressed_debounced(event_data->gpio_num)) {
        return;
    }
    
    if (menu_state.menu_active) {
        // Handle menu navigation
        if (event_data->gpio_num == BUTTON_UP_GPIO) {
            handle_menu_navigation();
        }
        else if (event_data->gpio_num == BUTTON_SELECT_GPIO) {
            handle_menu_selection();
        }
    }
    else if (menu_state.in_game) {
        // Handle in-game controls (return to menu)
        if (event_data->gpio_num == BUTTON_UP_GPIO) {
            return_to_menu();
        }
        // SELECT button could be used for game-specific actions
    }
}

// === BUTTON POLLING TASK (ALTERNATIVE TO ISR) ===

void button_polling_task(void* pvParameters)
{
    static bool last_up_state = false;
    static bool last_select_state = false;
    
    while (1) {
        // Read current button states (inverted because of pull-up)
        bool current_up_state = !gpio_get_level(BUTTON_UP_GPIO);
        bool current_select_state = !gpio_get_level(BUTTON_SELECT_GPIO);
        
        // Check for button press (transition from false to true)
        if (current_up_state && !last_up_state) {
            button_event_data_t event_data = {
                .gpio_num = BUTTON_UP_GPIO,
                .event = BUTTON_EVENT_PRESSED,
                .timestamp = xTaskGetTickCount()
            };
            xQueueSend(button_queue, &event_data, 0);
            ESP_LOGI("BUTTON", "UP button pressed");
        }
        
        if (current_select_state && !last_select_state) {
            button_event_data_t event_data = {
                .gpio_num = BUTTON_SELECT_GPIO,
                .event = BUTTON_EVENT_PRESSED,
                .timestamp = xTaskGetTickCount()
            };
            xQueueSend(button_queue, &event_data, 0);
            ESP_LOGI("BUTTON", "SELECT button pressed");
        }
        
        // Update last states
        last_up_state = current_up_state;
        last_select_state = current_select_state;
        
        // Poll every 50ms
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// === INITIALIZATION FUNCTIONS ===

esp_err_t init_menu_buttons(void)
{
    ESP_LOGI("MENU", "Initializing buttons...");
    
    // Create button event queue
    button_queue = xQueueCreate(10, sizeof(button_event_data_t));
    if (button_queue == NULL) {
        ESP_LOGE("MENU", "Failed to create button queue");
        return ESP_FAIL;
    }

    // Configure GPIO for buttons with pull-up resistors
    gpio_config_t gpio_conf = {
        .pin_bit_mask = (1ULL << BUTTON_UP_GPIO) | (1ULL << BUTTON_SELECT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE  // Start with polling, can enable ISR later
    };

    esp_err_t ret = gpio_config(&gpio_conf);
    if (ret != ESP_OK) {
        ESP_LOGE("MENU", "Failed to configure GPIO: %s", esp_err_to_name(ret));
        return ret;
    }

    // Option 1: Use ISR (comment out polling task if using this)
    /*
    gpio_conf.intr_type = GPIO_INTR_NEGEDGE;
    gpio_config(&gpio_conf);
    
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_UP_GPIO, button_isr_handler, (void*)BUTTON_UP_GPIO);
    gpio_isr_handler_add(BUTTON_SELECT_GPIO, button_isr_handler, (void*)BUTTON_SELECT_GPIO);
    */
    
    // Option 2: Use polling task (more reliable for debugging)
    xTaskCreate(button_polling_task, "button_poll", 2048, NULL, 3, NULL);

    ESP_LOGI("MENU", "Buttons initialized successfully");
    return ESP_OK;
}

void show_boot_screen(void)
{
    ssd1306_clear_buffer();
    
    // Boot animation
    ssd1306_draw_string(20, 15, "game console");
    ssd1306_draw_string(35, 30, "v1.0");
    ssd1306_draw_string(25, 45, "loading...");
    
    ssd1306_update_display();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

// === MAIN MENU TASK ===

void menu_task(void* pvParameters)
{
    button_event_data_t event_data;
    
    // Show boot screen
    show_boot_screen();
    
    // Draw initial menu
    draw_complete_menu();
    
    ESP_LOGI("MENU", "Menu system ready");
    
    while (1) {
        // Wait for button events with timeout
        if (xQueueReceive(button_queue, &event_data, pdMS_TO_TICKS(100)) == pdTRUE) {
            ESP_LOGI("MENU", "Button event received: GPIO %d", event_data.gpio_num);
            handle_button_event(&event_data);
        }
        
        // Small delay to prevent excessive CPU usage
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
