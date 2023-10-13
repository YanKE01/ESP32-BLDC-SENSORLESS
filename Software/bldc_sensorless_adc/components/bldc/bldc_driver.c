#include "bldc_driver.h"
#include "string.h"
#include "bldc_startup.h"
#include "bldc_adc.h"
#include "esp_log.h"
#include "bldc_common.h"

static const char *TAG = "bldc_driver";
bldc_t bldc;
uint8_t test_flag = 0;

#define LONG_PULSE_CNT 10 // 电容充电时间计数
#define SHORT_PULSE_CNT 4 // 脉冲时间计数

static bool mcpwm_timer_event_cb(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_data)
{
    static uint8_t status = 0;
    status = !status;
    gpio_set_level(bldc.driver->test_gpio.gpio_num, status);

    if (bldc.startup->align_flag == 0)
    {
        // 转子位置检测
        align_pos_check_process(bldc.driver, bldc.startup, bldc.parameter);
        if (bldc.startup->adc_flag == 1 || bldc.startup->charge_flag == 1)
        {
            bldc.startup->charge_count++;
        }

        if (bldc.startup->adc_flag == 1)
        {
            // 读取adc数值
            if (bldc.startup->pos_idx <= 5)
            {
                bldc.startup->adc_check_buf[bldc.startup->pos_idx] = bldc_adc_read_amp(bldc.adc);
            }
        }

        if (bldc.startup->charge_flag == 1)
        {
            if (bldc.startup->charge_count >= 5)
            {
                bldc.startup->charge_count = 0; // 充电清0
                all_mos_off(bldc.driver);
                bldc.startup->charge_flag = 0; // 充电标志清0
            }
        }
        else if (bldc.startup->adc_flag == 1)
        {
            if (bldc.startup->charge_count >= 5)
            {
                bldc.startup->charge_count = 0;
                all_mos_off(bldc.driver);
                bldc.startup->adc_flag = 0;
            }
        }
    }
    return false;
}

esp_err_t bldc_mcpwm_init(bldc_driver_t *driver, bldc_startup_parameter_t *startup, bldc_adc_t *adc, bldc_parameter_t *parameter)
{
    // init sem
    startup->info_sem = xSemaphoreCreateBinary();
    // init test io
    driver->test_gpio.io_conf.intr_type = GPIO_INTR_DISABLE;
    driver->test_gpio.io_conf.mode = GPIO_MODE_OUTPUT;
    driver->test_gpio.io_conf.pin_bit_mask = (1ULL << driver->test_gpio.gpio_num);
    driver->test_gpio.io_conf.pull_down_en = 0;
    driver->test_gpio.io_conf.pull_up_en = 0;
    gpio_config(&driver->test_gpio.io_conf);

    // init lin control io
    for (int i = 0; i < 3; ++i)
    {
        driver->lin_gpio.io_conf.intr_type = GPIO_INTR_DISABLE;
        driver->lin_gpio.io_conf.mode = GPIO_MODE_OUTPUT;
        driver->lin_gpio.io_conf.pin_bit_mask = (1ULL << driver->lin_gpio.lin_gpio_nums[i]);
        driver->lin_gpio.io_conf.pull_down_en = 0;
        driver->lin_gpio.io_conf.pull_up_en = 0;
        gpio_config(&driver->lin_gpio.io_conf);
    }

    // create mcpwm timer
    driver->mcpwm.timer = NULL;
    driver->mcpwm.timer_config.group_id = 0;
    driver->mcpwm.timer_config.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
    driver->mcpwm.timer_config.resolution_hz = 20 * 1000 * 1000;            // 20MHZ
    driver->mcpwm.timer_config.count_mode = MCPWM_TIMER_COUNT_MODE_UP_DOWN; // center mode
    driver->mcpwm.timer_config.period_ticks = 2000;                         // 2000*0.05us = 100us  10khz
    ESP_ERROR_CHECK(mcpwm_new_timer(&driver->mcpwm.timer_config, &driver->mcpwm.timer));

    // create mcpwm operator
    mcpwm_operator_config_t operator_config = {
        .group_id = 0,
    };
    for (int i = 0; i < 3; ++i)
    {
        ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &driver->mcpwm.operators[i]));
    }
    // connect operator to the same timer
    for (int i = 0; i < 3; ++i)
    {
        ESP_ERROR_CHECK(mcpwm_operator_connect_timer(driver->mcpwm.operators[i], driver->mcpwm.timer));
    }

    // create comparators
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    for (int i = 0; i < 3; ++i)
    {
        ESP_ERROR_CHECK(mcpwm_new_comparator(driver->mcpwm.operators[i], &comparator_config, &driver->mcpwm.comparators[i]));
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[i], 0));
    }

    // create mcpwm generators
    mcpwm_generator_config_t generator_config = {};
    for (int i = 0; i < 3; ++i)
    {
        generator_config.gen_gpio_num = driver->mcpwm.hin_gpio_nums[i];
        ESP_ERROR_CHECK(mcpwm_new_generator(driver->mcpwm.operators[i], &generator_config, &driver->mcpwm.generators[i]));
    }

    // set generator action
    for (int i = 0; i < 3; ++i)
    {
        ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(driver->mcpwm.generators[i],
                                                                     MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, driver->mcpwm.comparators[i], MCPWM_GEN_ACTION_LOW),
                                                                     MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, driver->mcpwm.comparators[i], MCPWM_GEN_ACTION_HIGH),
                                                                     MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    }

    // register callback
    mcpwm_timer_event_callbacks_t mcpwm_timer_event = {
        .on_full = mcpwm_timer_event_cb,
    };

    bldc.driver = driver;
    bldc.startup = startup;
    bldc.adc = adc;
    bldc.parameter = parameter;

    mcpwm_timer_register_event_callbacks(driver->mcpwm.timer, &mcpwm_timer_event, NULL);

    gpio_set_level(bldc.driver->test_gpio.gpio_num, 0);

    // enable mcpwm
    ESP_ERROR_CHECK(mcpwm_timer_enable(driver->mcpwm.timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(driver->mcpwm.timer, MCPWM_TIMER_START_NO_STOP));

    // set maxpwm duty
    driver->mcpwm.max_duty = driver->mcpwm.timer_config.period_ticks / 2;

    return ESP_OK;
}

void bldc_u_phase_test(bldc_driver_t *driver, bldc_adc_t *adc)
{

    for (int i = 0; i < 5000; i++)
    {
        // u上管导通
        mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], 0.9 * driver->mcpwm.max_duty);
        mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], 0);
        mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], 0);

        // v下管导通
        gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 1); // U
        gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 0); // v
        gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 1); // w
    }
    ESP_LOGI(TAG, "amp:%d", bldc_adc_read_amp(adc));

    // 关闭输出
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 1); // U
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 1); // v
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 1); // w
}

void bldc_turn(bldc_driver_t *driver, bldc_parameter_t *parameter)
{
    if (parameter->phase_cnt > 6)
    {
        parameter->phase_cnt = 1;
    }
    else if (parameter->phase_cnt < 1)
    {
        parameter->phase_cnt = 6;
    }

    switch (parameter->phase_cnt)
    {
    case 6:
        uh_vl(driver, parameter);
        break;
    case 5:
        uh_wl(driver, parameter);
        break;
    case 4:
        vh_wl(driver, parameter);
        break;
    case 3:
        vh_ul(driver, parameter);
        break;
    case 2:
        wh_ul(driver, parameter);
        break;
    case 1:
        wh_vl(driver, parameter);
        break;
    default:
        break;
    }
}