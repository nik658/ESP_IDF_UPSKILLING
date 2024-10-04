#include <stdio.h>
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h" 
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE 
#include "esp_log.h"

#define SWITCH_PIN 0 
#define LED_PIN 2   

static const char *TAG = "LED_Controller"; 

void app_main(void)
{
    
    esp_rom_gpio_pad_select_gpio(SWITCH_PIN);
    gpio_set_direction(SWITCH_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(SWITCH_PIN, GPIO_PULLUP_ONLY); // Enable pull-up resistor

  
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000, 
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num = LED_PIN, 
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);
    
    ledc_fade_func_install(0);

    int current_state = 0; 

    while (1)
    {
        bool current_switch_state = gpio_get_level(SWITCH_PIN);

        if (current_switch_state == 0) {
            current_state = ((current_state + 1) % 3); 
            ESP_LOGI(TAG, "Switch pressed, changing state to: %d", current_state);
            vTaskDelay(200 / portTICK_PERIOD_MS); 
        }

        switch (current_state) {
            case 0:
                ESP_LOGI(TAG, "Current State: ON");
                ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255);
                ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                break;

            case 1:
                ESP_LOGI(TAG, "Current State: OFF");
                 ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
                ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                break;

            case 2:
                ESP_LOGI(TAG, "Current State: FADE");
                for (int duty_cycle = 0; duty_cycle <= 255; duty_cycle++) {
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_cycle);
                    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                for (int duty_cycle = 255; duty_cycle >= 0; duty_cycle--) {
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_cycle);
                    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                break;

            default:
                break;
        }

        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
}