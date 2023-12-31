#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"
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
SemaphoreHandle_t xSemaphore; // 信号量句柄

/**
 * @description: app任务入口
 * @param {void} *args
 * @return {*}
 */
static void app_task(void *args)
{
    static uint8_t log_count = 0;
    static uint8_t lock_count = 0;
    static double change_count = 0.0f;

    while (1) {
        // 打印日志
        if (motorParameter.isStart == START && simpleOpen.runStep == 3) {
            log_count++;
            if (log_count == 100) {
                log_count = 0;
                ESP_LOGI(TAG, "-------------------------------------------------");
                ESP_LOGI(TAG, "System Speed:%d,Target Speed:%d,duty:%lu", hallLessParameter.speedRpm, speedPid.SetPoint, motorParameter.pwmDuty);
                ESP_LOGI(TAG, "Error:%.2f,P:%.2f,I:%.2f", speedPid.Error, speedPid.Up, speedPid.Ui);
                ESP_LOGI(TAG, "Error Hall:%d", hallLessParameter.errorHallLessCount);
                ESP_LOGI(TAG, "-------------------------------------------------\n");
            }
        }

        // 判断堵转标志是否成立
        if (motorParameter.lock) {
            lock_count++;
            if (lock_count == 10) {
                lock_count = 0;
                motorParameter.isStart = START;
                motorParameter.lock = 0;
                simpleOpen.runStep = 0;
                ESP_LOGI(TAG, "lock finished");
            }
        }

        // 改变风速
        if (simpleOpen.runStep == 3 && motorParameter.isStart == START) {
            switch (rmakerParameter.mode) {
            case NORMAL:
                // 直接速度控制
                if (speedPid.SetPoint != rmakerParameter.speed) {
                    speedPid.SetPoint = rmakerParameter.speed; // 刷新速度
                }
                break;
            case NATURAL:
                // 自然风控制
                change_count += 0.01f;
                speedPid.SetPoint = app_generate_noisy_speed(300, 800, 100, change_count);
                break;
            default:
                break;
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/**
 * @description: 硬件定时器回调
 * @param {gptimer_handle_t} timer
 * @param {gptimer_alarm_event_data_t} *edata
 * @param {void} *user_ctx
 * @return {*}
 */
static bool IRAM_ATTR bldc_main_loop_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    hal_bldc_main_loop();

    // PID运算
    if (simpleOpen.runStep == 3 && motorParameter.isStart == START) {
        // 当前已经进入无感状态 应该等到速度稳定后介入PID控制
        xSemaphoreGive(xSemaphore);
    }

    return pdTRUE;
}

/**
 * @description: PID运算.注：无法在isr中调用浮点运算
 * @param {void*} args
 * @return {*}
 */
void app_pid_task(void *args)
{
    while (1) {
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
            int motor_pwm_t = app_pid_operation(&speedPid, hallLessParameter.speedRpm);
            /* 最低速度限制 */
            if (motor_pwm_t > -200 && motor_pwm_t <= 0) {
                motor_pwm_t = -200;
            } else if (motor_pwm_t < 200 && motor_pwm_t > 0) {
                motor_pwm_t = 200;
            }

            motorParameter.pwmDuty = abs(motor_pwm_t);
        }
    }
}

void app_main(void)
{

    /* 外设初始化,软硬件. */
    app_rmaker_init();   // rainmaker初始化
    app_variable_init(); // 系统变量初始化
    app_pid_init();      // pid初始化
    hal_btn_init();      // 按键初始化
    hal_bldc_hal_init(); // bldc硬件初始化

    /* 创建BLDC主循环代码,硬件定时器. */
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = 50,                  // t=50us, f=1/t= 20khz
        .flags.auto_reload_on_alarm = true, // 使能重装载
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = bldc_main_loop_cb, // register user callback
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    /* rtos任务初始化. */
    xSemaphore = xSemaphoreCreateBinary();
    xTaskCreate(app_task, "app_task", 1024 * 4, NULL, 4, NULL);
    xTaskCreate(app_pid_task, "app_pid_task", 1024 * 4, NULL, 5, NULL);
}