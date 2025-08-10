#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Definições dos botões
#define BUTTON_1_GPIO 40  // Botão verde
#define BUTTON_2_GPIO 38  // Botão vermelho

/**
 * @brief Inicializa os botões com pull-up interno
 */
void button_init(void);

/**
 * @brief Lê o estado de um botão com debounce
 * @param gpio_num Número do GPIO do botão
 * @return true se o botão está pressionado, false caso contrário
 */
bool button_read(gpio_num_t gpio_num);

/**
 * @brief Aguarda até que um botão seja pressionado
 * @param gpio_num Número do GPIO do botão
 * @param timeout_ms Tempo máximo de espera em ms
 * @return true se o botão foi pressionado, false se timeout ocorreu
 */
bool button_wait_press(gpio_num_t gpio_num, uint32_t timeout_ms);

#endif // BUTTON_H