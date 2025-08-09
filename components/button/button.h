#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

// ========================= Constantes ========================= //
#define BTN_DEBOUNCE_MS      50
#define BTN_LONG_PRESS_MS    1000
#define BTN_MAX_SUPPORTED    8

// ========================= Tipos ============================== //
typedef enum {
    BTN_EVT_PRESSED,
    BTN_EVT_RELEASED,
    BTN_EVT_LONG_PRESS,
    BTN_EVT_DOUBLE_CLICK
} btn_event_t;

typedef struct {
    gpio_num_t pin;
    btn_event_t type;
    uint32_t timestamp;
} btn_event_info_t;

typedef void (*btn_isr_cb_t)(gpio_num_t pin, btn_event_t event_type);

typedef struct {
    gpio_num_t pin;
    gpio_pull_mode_t pull;
    bool active_low;                   // true se o botão for ativo em nível baixo
    uint32_t debounce_ms;
    uint32_t long_press_ms;
    btn_isr_cb_t callback;
} btn_config_t;

// =================== Inicialização =========================== //
esp_err_t btn_init_simple(gpio_config_t* config);
esp_err_t btn_init_with_callback(gpio_config_t* config, btn_isr_cb_t callback);
esp_err_t btn_init_custom(btn_config_t* cfg);
esp_err_t btn_init_pullup(gpio_num_t pin, btn_isr_cb_t callback);
esp_err_t btn_init_pulldown(gpio_num_t pin, btn_isr_cb_t callback);

// =================== Leitura ================================ //
int btn_read_raw(gpio_num_t pin);
int btn_read_filtered(gpio_num_t pin);

// =================== Eventos ================================ //
esp_err_t btn_wait_for_event(btn_event_info_t* evt, uint32_t timeout_ms);
esp_err_t btn_get_latest_event(btn_event_info_t* evt);

// =================== Controle =============================== //
esp_err_t btn_set_enabled(gpio_num_t pin, bool enabled);
esp_err_t btn_clear_event_queue(void);
esp_err_t btn_shutdown_all(void);

#endif // BUTTON_H
