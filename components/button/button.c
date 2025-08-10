#include "button/button.h"

static const char *TAG = "BUTTON";
static uint32_t last_press_time[64] = {0};
static const uint32_t debounce_time_ms = 50;

void button_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_1_GPIO) | (1ULL << BUTTON_2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao configurar botões: %s", esp_err_to_name(ret));
    }
    
    ESP_LOGI(TAG, "Botões inicializados (GPIO %d e %d)", BUTTON_1_GPIO, BUTTON_2_GPIO);
}

bool button_read(gpio_num_t gpio_num) {
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Verifica se o botão está pressionado (nível baixo com pull-up)
    if (gpio_get_level(gpio_num) == 0) {
        // Verifica debounce
        if ((now - last_press_time[gpio_num]) > debounce_time_ms) {
            last_press_time[gpio_num] = now;
            return true;
        }
    }
    return false;
}

bool button_wait_press(gpio_num_t gpio_num, uint32_t timeout_ms) {
    uint32_t start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    while ((xTaskGetTickCount() * portTICK_PERIOD_MS - start_time) < timeout_ms) {
        if (button_read(gpio_num)) {
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return false;
}