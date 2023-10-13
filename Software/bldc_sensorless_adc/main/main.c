#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "bldc.h"
#include "bldc_driver.h"
#include "bldc_startup.h"
#include "bldc_adc.h"

const char *TAG = "APP_MAIN";

bldc_driver_t bldc_driver;
bldc_adc_t bldc_adc;
bldc_startup_parameter_t bldc_startup;
bldc_parameter_t bldc_parameter;

void info_task(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(bldc_startup.info_sem, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "FINISHED:%d", bldc_startup.phase_cnt);
            for (int i = 0; i < 6; i++)
            {
                ESP_LOGI(TAG, "%d:%d", i, bldc_startup.adc_check_buf[i]);
            }
        }
    }
}

void debug_task(void *pvParameters)
{
    while (1)
    {
        if (bldc_startup.align_flag == 1)
        {
            // bldc_startup.align_flag = 0;
            // bldc_startup.adc_flag = 0;
            // bldc_startup.charge_flag = 0;
            // bldc_startup.pos_idx = 0;
            // bldc_startup.pos_check_stage = 0;
            // bldc_startup.charge_count = 0;
            // bldc_startup.phase_cnt = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP BLDC ADC DEVKIT");
    bldc_driver.lin_gpio.lin_gpio_nums[u_phase] = GPIO_NUM_12;
    bldc_driver.lin_gpio.lin_gpio_nums[v_phase] = GPIO_NUM_11;
    bldc_driver.lin_gpio.lin_gpio_nums[w_phase] = GPIO_NUM_10;
    bldc_driver.test_gpio.gpio_num = GPIO_NUM_40;
    bldc_driver.mcpwm.hin_gpio_nums[u_phase] = GPIO_NUM_17;
    bldc_driver.mcpwm.hin_gpio_nums[v_phase] = GPIO_NUM_16;
    bldc_driver.mcpwm.hin_gpio_nums[w_phase] = GPIO_NUM_15;

    memset(&bldc_startup, 0, sizeof(bldc_startup));
    bldc_adc_init(&bldc_adc);
    bldc_mcpwm_init(&bldc_driver, &bldc_startup, &bldc_adc,&bldc_parameter);
    xTaskCreate(info_task, "info_task", 4096, NULL, 3, NULL);
    xTaskCreate(debug_task, "debug_task", 4096, NULL, 2, NULL);
}