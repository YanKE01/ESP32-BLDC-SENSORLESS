#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal_bldc.h"
#include "app_variable.h"
#include "app_pid.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "iot_button.h"

const char *TAG = "BLDC";
int motor_pwm_s = 0;
static button_handle_t btn_handle;

static void button_single_click_cb(void *arg, void *data)
{
    motorParameter.isStart = !motorParameter.isStart;

    ESP_LOGI(TAG, "STATE:%d", motorParameter.isStart);
}
/**
 * @description: bldc main loop
 * @param {void} *arg
 * @return {*}
 */
static void periodic_timer_callback(void *arg)
{
    hal_bldc_main_loop();
    if (simpleOpen.runStep == 3 && motorParameter.isStart == START && hallLessParameter.stableFlag)
    {
        // 当前已经进入无感状态 应该等到速度稳定后介入PID控制

        motor_pwm_s = app_pid_operation(&speedPid, hallLessParameter.speedRpm);

        /* 最低速度限制 */
        if (motor_pwm_s > -200 && motor_pwm_s <= 0)
        {
            motor_pwm_s = -200;
        }
        else if (motor_pwm_s < 200 && motor_pwm_s > 0)
        {
            motor_pwm_s = 200;
        }

        motorParameter.pwmDuty = abs(motor_pwm_s);
    }
}

static void app_task(void *args)
{
    static uint8_t log_count = 0;
    static int speedPrev = 0;
    while (1)
    {
        // 打印日志
        if (log_count++ == 10)
        {
            if (motorParameter.isStart == START)
            {
                ESP_LOGI(TAG, "System Speed:%d,Target Speed:%.2f,duty:%lu", hallLessParameter.speedRpm, speedPid.SetPoint, motorParameter.pwmDuty);
                ESP_LOGI(TAG, "Error:%.2f,P:%.2f,I:%.2f", speedPid.Error, speedPid.Up, speedPid.Ui);
            }
            log_count = 0;
        }

        // 判断速度是否稳定
        if (motorParameter.isStart == START && simpleOpen.runStep == 3 && hallLessParameter.stableFlag == 0)
        {
            if (abs(speedPrev - hallLessParameter.speedRpm) <= 5)
            {
                hallLessParameter.stableFlag = 1;
                ESP_LOGI(TAG, "Error:%.2f,P:%.2f,I:%.2f", speedPid.Error, speedPid.Up, speedPid.Ui);
            }
            speedPrev = hallLessParameter.speedRpm;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    app_variable_init();
    hal_bldc_hal_init();
    app_pid_init();
    button_config_t cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 2000,
        .short_press_time = 500,
        .gpio_button_config = {
            .gpio_num = GPIO_NUM_0,
            .active_level = 0,
        },
    };

    btn_handle = iot_button_create(&cfg);
    iot_button_register_cb(btn_handle, BUTTON_SINGLE_CLICK, button_single_click_cb, NULL);

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        /* name is optional, but may help identify the timer when debugging */
        .name = "periodic",
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 50)); // 20KHZ

    xTaskCreate(app_task, "app_task", 1024 * 4, NULL, 5, NULL);
}