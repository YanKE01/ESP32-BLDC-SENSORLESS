#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "app_wind.h"
#include "app_variable.h"
#include "app_pid.h"
#include "app_rmaker.h"
#include "app_wifi.h"
#include "hal_btn.h"
#include "hal_bldc.h"

const char *TAG = "BLDC MAIN";

/**
 * @description: bldc主循环
 * @param {void} *arg
 * @return {*}
 */
static void periodic_timer_callback(void *arg)
{
    hal_bldc_main_loop();
    if (simpleOpen.runStep == 3 && motorParameter.isStart == START)
    {
        // 当前已经进入无感状态 应该等到速度稳定后介入PID控制
        int motor_pwm_s = app_pid_operation(&speedPid, hallLessParameter.speedRpm);

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

/**
 * @description: app任务入口
 * @param {void} *args
 * @return {*}
 */
static void app_task(void *args)
{
    static uint8_t log_count = 0;
    static uint8_t lock_count = 0;
    static uint8_t change_count = 0;

    while (1)
    {
        // 打印日志
        if (motorParameter.isStart == START && simpleOpen.runStep == 3)
        {
            log_count++;
            if (log_count == 100)
            {
                log_count = 0;
                ESP_LOGI(TAG, "System Speed:%d,Target Speed:%.2f,duty:%lu", hallLessParameter.speedRpm, speedPid.SetPoint, motorParameter.pwmDuty);
                ESP_LOGI(TAG, "Error:%.2f,P:%.2f,I:%.2f", speedPid.Error, speedPid.Up, speedPid.Ui);
            }
        }

        // 判断堵转标志是否成立
        if (motorParameter.lock)
        {
            lock_count++;
            if (lock_count == 10)
            {
                lock_count = 0;
                motorParameter.isStart = START;
                motorParameter.lock = 0;
                simpleOpen.runStep = 0;
                ESP_LOGI(TAG, "lock finished");
            }
        }

        // 改变风速
        if (simpleOpen.runStep == 3 && motorParameter.isStart == START)
        {
            // speedPid.SetPoint = temp_speed;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // app_rmaker_init();   // rainmaker初始化
    app_variable_init(); // 系统变量初始化
    app_pid_init();      // pid初始化
    hal_btn_init();      // 按键初始化
    hal_bldc_hal_init(); // bldc硬件初始化

    // 硬件定时器：无感BLDC主循环 20KHZ
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        .name = "periodic",
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 100)); // 20KHZ

    // 应用层任务
    xTaskCreate(app_task, "app_task", 1024 * 4, NULL, 3, NULL);
}