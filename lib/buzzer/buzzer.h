#ifndef BUZZER_DRV_H
#define BUZZER_DRV_H

#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"

// Tempo padrão de beep em milissegundos
#define BUZZER_DEFAULT_BEEP_DURATION_MS 200

// Estrutura de configuração do buzzer
typedef struct {
    gpio_num_t pin;                   // GPIO conectado ao buzzer
    ledc_channel_t channel;          // Canal LEDC usado para PWM
    ledc_timer_t timer;              // Timer LEDC usado para PWM
    uint32_t frequency_hz;           // Frequência do som (em Hz)
    ledc_mode_t speed_mode;          // Modo de velocidade do LEDC
    ledc_timer_bit_t duty_resolution;// Resolução do duty cycle
    uint32_t duty;                   // Duty cycle padrão
} buzzer_config_t;

/**
 * @brief Inicializa o buzzer com a configuração especificada.
 *
 * @param config Ponteiro para a configuração do buzzer.
 * @return esp_err_t ESP_OK em caso de sucesso, erro caso contrário.
 */
esp_err_t buzzer_init(const buzzer_config_t *config);

/**
 * @brief Emite um beep com a duração e duty especificados.
 *
 * @param duration_ms Duração do beep em milissegundos.
 * @param duty Opcional: novo duty cycle (ou 0 para manter o atual).
 * @return esp_err_t ESP_OK em caso de sucesso, erro caso contrário.
 */
esp_err_t buzzer_beep(uint32_t duration_ms, uint32_t duty);

/**
 * @brief Altera a frequência do buzzer em tempo real.
 *
 * @param frequency_hz Nova frequência em Hz.
 * @return esp_err_t ESP_OK em caso de sucesso, erro caso contrário.
 */
esp_err_t buzzer_set_frequency(uint32_t frequency_hz);

/**
 * @brief Liga o buzzer continuamente com o duty atual.
 *
 * @return esp_err_t ESP_OK em caso de sucesso, erro caso contrário.
 */
esp_err_t buzzer_on(void);

/**
 * @brief Desliga o buzzer.
 *
 * @return esp_err_t ESP_OK em caso de sucesso, erro caso contrário.
 */
esp_err_t buzzer_off(void);

#endif // BUZZER_DRV_H
