#include "button.h"

static const char* TAG = "BUTTONS";

static QueueHandle_t button_event_queue = NULL;
static button_config_t button_configs[MAX_BUTTONS];
static int num_buttons_configured = 0;
static bool buttons_initialized = false;

static void IRAM_ATTR button_isr_handler(void* arg) {
    gpio_num_t gpio_num = (gpio_num_t)(int)arg;
    button_event_data_t event_data = {
        .gpio_num = gpio_num,
        .event = BUTTON_EVENT_PRESSED,
        .timestamp = esp_timer_get_time() / 1000 // em ms
    };
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(button_event_queue, &event_data, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

esp_err_t init_buttons(gpio_config_t* gpio_button_config) {
    if (gpio_button_config == NULL) {
        ESP_LOGE(TAG, "GPIO config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Configurar GPIOs
    esp_err_t ret = gpio_config(gpio_button_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO: %s", esp_err_to_name(ret));
        return ret;
    }
    
    buttons_initialized = true;
    ESP_LOGI(TAG, "Buttons initialized successfully");
    return ESP_OK;
}

esp_err_t init_buttons_isr(gpio_config_t* gpio_button_config, button_isr_callback_t isr_callback) {
    if (gpio_button_config == NULL) {
        ESP_LOGE(TAG, "GPIO config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Criar fila de eventos se não existir
    if (button_event_queue == NULL) {
        button_event_queue = xQueueCreate(10, sizeof(button_event_data_t));
        if (button_event_queue == NULL) {
            ESP_LOGE(TAG, "Failed to create event queue");
            return ESP_ERR_NO_MEM;
        }
    }
    
    // Instalar driver de ISR se necessário
    static bool isr_service_installed = false;
    if (!isr_service_installed) {
        esp_err_t ret = gpio_install_isr_service(0);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "Failed to install ISR service: %s", esp_err_to_name(ret));
            return ret;
        }
        isr_service_installed = true;
    }
    
    // Configurar GPIOs
    esp_err_t ret = gpio_config(gpio_button_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Configurar ISR para cada GPIO na máscara
    for (int i = 0; i < 64; i++) {
        if (gpio_button_config->pin_bit_mask & (1ULL << i)) {
            ret = gpio_isr_handler_add(i, button_isr_handler, (void*)i);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to add ISR handler for GPIO %d: %s", i, esp_err_to_name(ret));
                return ret;
            }
        }
    }
    
    buttons_initialized = true;
    ESP_LOGI(TAG, "Buttons with ISR initialized successfully");
    return ESP_OK;
}

esp_err_t button_config_advanced(button_config_t* config) {
    if (config == NULL) {
        ESP_LOGE(TAG, "Button config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (num_buttons_configured >= MAX_BUTTONS) {
        ESP_LOGE(TAG, "Maximum number of buttons reached");
        return ESP_ERR_NO_MEM;
    }
    
    // Configurar GPIO individual
    gpio_config_t gpio_conf = {
        .pin_bit_mask = (1ULL << config->gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = (config->pull_mode == GPIO_PULLUP_ONLY) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = (config->pull_mode == GPIO_PULLDOWN_ONLY) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    
    esp_err_t ret = gpio_config(&gpio_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO %d: %s", config->gpio_num, esp_err_to_name(ret));
        return ret;
    }
    
    // Salvar configuração
    button_configs[num_buttons_configured] = *config;
    num_buttons_configured++;
    
    ESP_LOGI(TAG, "Button GPIO %d configured successfully", config->gpio_num);
    return ESP_OK;
}

int button_read(gpio_num_t gpio_num) {
    if (!buttons_initialized) {
        ESP_LOGW(TAG, "Buttons not initialized");
        return -1;
    }
    
    return gpio_get_level(gpio_num);
}

int button_read_debounced(gpio_num_t gpio_num) {
    static uint32_t last_read_time[64] = {0};
    static int last_state[64] = {-1};
    
    if (!buttons_initialized) {
        ESP_LOGW(TAG, "Buttons not initialized");
        return -1;
    }
    
    uint32_t current_time = esp_timer_get_time() / 1000; // em ms
    int current_state = gpio_get_level(gpio_num);
    
    // Se mudou de estado e passou tempo suficiente
    if (current_state != last_state[gpio_num] && 
        (current_time - last_read_time[gpio_num]) > BUTTON_DEBOUNCE_TIME_MS) {
        
        last_state[gpio_num] = current_state;
        last_read_time[gpio_num] = current_time;
        return current_state;
    }
    
    return last_state[gpio_num];
}

esp_err_t button_wait_event(button_event_data_t* event_data, uint32_t timeout_ms) {
    if (event_data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (button_event_queue == NULL) {
        ESP_LOGE(TAG, "Event queue not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    TickType_t timeout_ticks = (timeout_ms == UINT32_MAX) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    
    if (xQueueReceive(button_event_queue, event_data, timeout_ticks) == pdTRUE) {
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

esp_err_t button_get_event(button_event_data_t* event_data) {
    if (event_data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (button_event_queue == NULL) {
        ESP_LOGE(TAG, "Event queue not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (xQueueReceive(button_event_queue, event_data, 0) == pdTRUE) {
        return ESP_OK;
    }
    
    return ESP_ERR_NOT_FOUND;
}

esp_err_t button_enable(gpio_num_t gpio_num, bool enable) {
    if (!buttons_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (enable) {
        return gpio_intr_enable(gpio_num);
    } else {
        return gpio_intr_disable(gpio_num);
    }
}

esp_err_t button_clear_events(void) {
    if (button_event_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xQueueReset(button_event_queue);
    return ESP_OK;
}

esp_err_t buttons_deinit(void) {
    // Remover handlers ISR
    for (int i = 0; i < num_buttons_configured; i++) {
        gpio_isr_handler_remove(button_configs[i].gpio_num);
    }
    
    // Limpar fila
    if (button_event_queue != NULL) {
        vQueueDelete(button_event_queue);
        button_event_queue = NULL;
    }
    
    num_buttons_configured = 0;
    buttons_initialized = false;
    
    ESP_LOGI(TAG, "Buttons deinitialized");
    return ESP_OK;
}

esp_err_t button_init_pullup(gpio_num_t gpio_num, button_isr_callback_t callback) {
    gpio_config_t gpio_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE  // Ativo baixo
    };
    
    if (callback != NULL) {
        return init_buttons_isr(&gpio_conf, callback);
    } else {
        return init_buttons(&gpio_conf);
    }
}

esp_err_t button_init_pulldown(gpio_num_t gpio_num, button_isr_callback_t callback) {
    gpio_config_t gpio_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE  // Ativo alto
    };
    
    if (callback != NULL) {
        return init_buttons_isr(&gpio_conf, callback);
    } else {
        return init_buttons(&gpio_conf);
    }
}