#include "button.h"

static const char* TAG = "BTN";

// ================= Variáveis internas =================
static QueueHandle_t btn_event_queue = NULL;
static btn_config_t btn_configs[BTN_MAX_SUPPORTED];
static int btn_count = 0;
static bool btn_initialized = false;

// ================= ISR =================
static void IRAM_ATTR btn_isr_handler(void* arg) {
    gpio_num_t pin = (gpio_num_t)(int)arg;
    btn_event_info_t evt = {
        .pin = pin,
        .type = BTN_EVT_PRESSED,
        .timestamp = esp_timer_get_time() / 1000
    };

    BaseType_t task_woken = pdFALSE;
    xQueueSendFromISR(btn_event_queue, &evt, &task_woken);

    if (task_woken) {
        portYIELD_FROM_ISR();
    }
}

// ================= Inicialização =================
esp_err_t btn_init_simple(gpio_config_t* config) {
    if (!config) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = gpio_config(config);
    if (ret != ESP_OK) return ret;
    btn_initialized = true;
    return ESP_OK;
}

esp_err_t btn_init_with_callback(gpio_config_t* config, btn_isr_cb_t callback) {
    if (!config) return ESP_ERR_INVALID_ARG;

    if (!btn_event_queue) {
        btn_event_queue = xQueueCreate(10, sizeof(btn_event_info_t));
        if (!btn_event_queue) return ESP_ERR_NO_MEM;
    }

    static bool isr_installed = false;
    if (!isr_installed) {
        esp_err_t ret = gpio_install_isr_service(0);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;
        isr_installed = true;
    }

    esp_err_t ret = gpio_config(config);
    if (ret != ESP_OK) return ret;

    for (int i = 0; i < 64; i++) {
        if (config->pin_bit_mask & (1ULL << i)) {
            ret = gpio_isr_handler_add(i, btn_isr_handler, (void*)i);
            if (ret != ESP_OK) return ret;
        }
    }

    btn_initialized = true;
    return ESP_OK;
}

esp_err_t btn_init_custom(btn_config_t* cfg) {
    if (!cfg || btn_count >= BTN_MAX_SUPPORTED) return ESP_ERR_INVALID_ARG;

    gpio_config_t conf = {
        .pin_bit_mask = (1ULL << cfg->pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = (cfg->pull == GPIO_PULLUP_ONLY),
        .pull_down_en = (cfg->pull == GPIO_PULLDOWN_ONLY),
        .intr_type = GPIO_INTR_ANYEDGE
    };

    esp_err_t ret = gpio_config(&conf);
    if (ret != ESP_OK) return ret;

    btn_configs[btn_count++] = *cfg;
    return ESP_OK;
}

esp_err_t btn_init_pullup(gpio_num_t pin, btn_isr_cb_t cb) {
    gpio_config_t conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    return cb ? btn_init_with_callback(&conf, cb) : btn_init_simple(&conf);
}

esp_err_t btn_init_pulldown(gpio_num_t pin, btn_isr_cb_t cb) {
    gpio_config_t conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE
    };
    return cb ? btn_init_with_callback(&conf, cb) : btn_init_simple(&conf);
}

// ================= Leitura =================
int btn_read_raw(gpio_num_t pin) {
    if (!btn_initialized) return -1;
    return gpio_get_level(pin);
}

int btn_read_filtered(gpio_num_t pin) {
    static uint32_t last_time[64] = {0};
    static int last_val[64] = {-1};

    if (!btn_initialized) return -1;

    uint32_t now = esp_timer_get_time() / 1000;
    int state = gpio_get_level(pin);

    if (state != last_val[pin] && (now - last_time[pin]) > BTN_DEBOUNCE_MS) {
        last_val[pin] = state;
        last_time[pin] = now;
        return state;
    }

    return last_val[pin];
}

// ================= Eventos =================
esp_err_t btn_wait_for_event(btn_event_info_t* evt, uint32_t timeout_ms) {
    if (!evt || !btn_event_queue) return ESP_ERR_INVALID_ARG;
    TickType_t ticks = (timeout_ms == UINT32_MAX) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xQueueReceive(btn_event_queue, evt, ticks) ? ESP_OK : ESP_ERR_TIMEOUT;
}

esp_err_t btn_get_latest_event(btn_event_info_t* evt) {
    if (!evt || !btn_event_queue) return ESP_ERR_INVALID_ARG;
    return xQueueReceive(btn_event_queue, evt, 0) ? ESP_OK : ESP_ERR_NOT_FOUND;
}

// ================= Controle =================
esp_err_t btn_set_enabled(gpio_num_t pin, bool enabled) {
    if (!btn_initialized) return ESP_ERR_INVALID_STATE;
    return enabled ? gpio_intr_enable(pin) : gpio_intr_disable(pin);
}

esp_err_t btn_clear_event_queue(void) {
    if (!btn_event_queue) return ESP_ERR_INVALID_STATE;
    xQueueReset(btn_event_queue);
    return ESP_OK;
}

esp_err_t btn_shutdown_all(void) {
    for (int i = 0; i < btn_count; i++) {
        gpio_isr_handler_remove(btn_configs[i].pin);
    }
    if (btn_event_queue) {
        vQueueDelete(btn_event_queue);
        btn_event_queue = NULL;
    }
    btn_count = 0;
    btn_initialized = false;
    return ESP_OK;
}
