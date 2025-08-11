#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"

#define BUTTON_DEBOUNCE_TIME_MS     50
#define BUTTON_LONG_PRESS_TIME_MS   1000
#define MAX_BUTTONS                 8

typedef enum {
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED,
    BUTTON_EVENT_LONG_PRESS,
    BUTTON_EVENT_DOUBLE_CLICK
} button_event_t;

typedef struct {
    gpio_num_t gpio_num;
    button_event_t event;
    uint32_t timestamp;
} button_event_data_t;

typedef void (*button_isr_callback_t)(gpio_num_t gpio_num, button_event_t event);

typedef struct {
    gpio_num_t gpio_num;
    gpio_pull_mode_t pull_mode;
    bool active_low;                    // true se botão é ativo em nível baixo
    uint32_t debounce_time_ms;
    uint32_t long_press_time_ms;
    button_isr_callback_t callback;
} button_config_t;

esp_err_t init_buttons(gpio_config_t* gpio_button_config);


esp_err_t init_buttons_isr(gpio_config_t* gpio_button_config, button_isr_callback_t isr_callback);

esp_err_t button_config_advanced(button_config_t* config);

int button_read(gpio_num_t gpio_num);

int button_read_debounced(gpio_num_t gpio_num);

esp_err_t button_wait_event(button_event_data_t* event_data, uint32_t timeout_ms);

esp_err_t button_get_event(button_event_data_t* event_data);

esp_err_t button_enable(gpio_num_t gpio_num, bool enable);

esp_err_t button_clear_events(void);

esp_err_t buttons_deinit(void);

esp_err_t button_init_pullup(gpio_num_t gpio_num, button_isr_callback_t callback);

esp_err_t button_init_pulldown(gpio_num_t gpio_num, button_isr_callback_t callback);

#endif // BUTTONS_H