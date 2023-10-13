#pragma once

#include "driver/mcpwm_prelude.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

enum bldc_phase
{
    u_phase = 0,
    v_phase,
    w_phase,
};

typedef struct mcpwm_config
{
    mcpwm_timer_handle_t timer;
    mcpwm_timer_config_t timer_config;
    mcpwm_oper_handle_t operators[3];
    mcpwm_cmpr_handle_t comparators[3];
    mcpwm_gen_handle_t generators[3];
    uint16_t max_duty;
    int hin_gpio_nums[3]; // upper  io
} mcpwm_config_t;

typedef struct lin_gpio
{
    gpio_config_t io_conf;
    int lin_gpio_nums[3]; // lower io
} lin_gpio_t;

typedef struct test_gpio
{
    gpio_config_t io_conf;
    int gpio_num; // test io
} test_gpio_t;

typedef struct bldc_driver
{
    lin_gpio_t lin_gpio;
    test_gpio_t test_gpio;
    mcpwm_config_t mcpwm;
} bldc_driver_t;

typedef struct bldc_startup_parameter
{
    uint8_t align_flag;
    uint8_t adc_flag;        // adc触发标志位
    uint8_t charge_flag;     // 电容充电标志位
    uint8_t pos_idx;         // 第inx个脉冲
    uint8_t pos_check_stage; // 脉冲注入阶段
    uint8_t charge_count;    // 充电计数
    uint8_t phase_cnt;
    uint8_t init_stage; // 初始位置
    int adc_check_buf[6];
    SemaphoreHandle_t info_sem; // 通知信号量
} bldc_startup_parameter_t;

typedef struct bldc_adc
{
    /* data */
    adc_oneshot_unit_handle_t adc1_handle;
} bldc_adc_t;

typedef struct bldc_parameter
{
    uint8_t phase_cnt;
    uint16_t pwm_duty;
} bldc_parameter_t;

typedef struct bldc
{
    bldc_startup_parameter_t *startup;
    bldc_driver_t *driver;
    bldc_adc_t *adc;
    bldc_parameter_t *parameter;
} bldc_t;
