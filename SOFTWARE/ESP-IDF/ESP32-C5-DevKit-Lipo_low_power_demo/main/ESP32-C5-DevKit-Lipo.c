// main/main.c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_log.h"

#define TAG "APP"


#define LED   GPIO_NUM_27      
#define BUT_PIN    GPIO_NUM_28     

void enter_deep_sleep_only_reset_wakes(void)
{
    
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

   
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,    ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RC_FAST,  ESP_PD_OPTION_OFF);
  
    //vTaskDelay(pdMS_TO_TICKS(50));

    esp_deep_sleep_start();
}

void app_main(void)
{

    esp_rom_gpio_pad_select_gpio(LED);
    esp_rom_gpio_pad_select_gpio(BUT_PIN);

	gpio_set_direction(LED, GPIO_MODE_OUTPUT);
	gpio_set_direction(BUT_PIN, GPIO_MODE_INPUT);
    vTaskDelay(pdMS_TO_TICKS(100));

    while (gpio_get_level(BUT_PIN) != 0) {
        gpio_set_level(LED, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    gpio_set_level(LED, 1);
    enter_deep_sleep_only_reset_wakes();

    // You should not meet this point. 
    while (1) { vTaskDelay(portMAX_DELAY); }
}

