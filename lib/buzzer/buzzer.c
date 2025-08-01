#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static buzzer_config_t buzzer_settings;

esp_err_t buzzer_init(const buzzer_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    buzzer_settings = *config;

    ledc_timer_config_t timer_conf = {
        .duty_resolution = buzzer_settings.duty_resolution,
        .freq_hz = buzzer_settings.frequency_hz,
        .speed_mode = buzzer_settings.speed_mode,
        .timer_num = buzzer_settings.timer,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    ledc_channel_config_t channel_conf = {
        .channel    = buzzer_settings.channel,
        .duty       = 0,
        .gpio_num   = buzzer_settings.pin,
        .speed_mode = buzzer_settings.speed_mode,
        .hpoint     = 0,
        .timer_sel  = buzzer_settings.timer
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));

    return ESP_OK;
}

esp_err_t buzzer_beep(uint32_t duration_ms, uint32_t duty)
{
    uint32_t actual_duty = (duty > 0) ? duty : buzzer_settings.duty;

    ESP_ERROR_CHECK(ledc_set_duty(
        buzzer_settings.speed_mode,
        buzzer_settings.channel,
        actual_duty
    ));

    ESP_ERROR_CHECK(ledc_update_duty(
        buzzer_settings.speed_mode,
        buzzer_settings.channel
    ));

    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    ESP_ERROR_CHECK(ledc_set_duty(
        buzzer_settings.speed_mode,
        buzzer_settings.channel,
        0
    ));

    ESP_ERROR_CHECK(ledc_update_duty(
        buzzer_settings.speed_mode,
        buzzer_settings.channel
    ));

    return ESP_OK;
}

esp_err_t buzzer_set_frequency(uint32_t frequency_hz)
{
    buzzer_settings.frequency_hz = frequency_hz;

    return ledc_set_freq(
        buzzer_settings.speed_mode,
        buzzer_settings.timer,
        frequency_hz
    );
}

esp_err_t buzzer_on(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(
        buzzer_settings.speed_mode,
        buzzer_settings.channel,
        buzzer_settings.duty
    ));

    ESP_ERROR_CHECK(ledc_update_duty(
        buzzer_settings.speed_mode,
        buzzer_settings.channel
    ));

    return ESP_OK;
}

esp_err_t buzzer_off(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(
        buzzer_settings.speed_mode,
        buzzer_settings.channel,
        0
    ));

    ESP_ERROR_CHECK(ledc_update_duty(
        buzzer_settings.speed_mode,
        buzzer_settings.channel
    ));

    return ESP_OK;
}
