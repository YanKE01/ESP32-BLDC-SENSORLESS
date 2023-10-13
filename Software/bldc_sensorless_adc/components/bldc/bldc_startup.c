#include "bldc_startup.h"
#include "bldc_driver.h"
#include "esp_log.h"
#include "rom/ets_sys.h"

void uv_w_phase_inject(bldc_driver_t *driver, bldc_startup_parameter_t *startup)
{
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], (uint32_t)(0.9 * driver->mcpwm.max_duty));
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], (uint32_t)(0.9 * driver->mcpwm.max_duty));
    startup->adc_flag = 1;
    startup->pos_idx = 0;
}

void w_uv_phase_inject(bldc_driver_t *driver, bldc_startup_parameter_t *startup)
{
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], (uint32_t)(0.9 * driver->mcpwm.max_duty));
    startup->adc_flag = 1;
    startup->pos_idx = 1;
}

void wu_v_phase_inject(bldc_driver_t *driver, bldc_startup_parameter_t *startup)
{
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], (uint32_t)(0.9 * driver->mcpwm.max_duty));
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], (uint32_t)(0.9 * driver->mcpwm.max_duty));
    startup->adc_flag = 1;
    startup->pos_idx = 2;
}

void v_wu_phase_inject(bldc_driver_t *driver, bldc_startup_parameter_t *startup)
{
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], (uint32_t)(0.9 * driver->mcpwm.max_duty));
    startup->adc_flag = 1;
    startup->pos_idx = 3;
}

void vw_u_phase_inject(bldc_driver_t *driver, bldc_startup_parameter_t *startup)
{
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], (uint32_t)(0.9 * driver->mcpwm.max_duty));
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], (uint32_t)(0.9 * driver->mcpwm.max_duty));
    startup->adc_flag = 1;
    startup->pos_idx = 4;
}

void u_vw_phase_inject(bldc_driver_t *driver, bldc_startup_parameter_t *startup)
{
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], (uint32_t)(0.9 * driver->mcpwm.max_duty));
    startup->adc_flag = 1;
    startup->pos_idx = 5;
}

/**
 * @brief 电容充电，下官全关
 *
 * @param driver
 * @param startup
 */
void capacitor_charge(bldc_driver_t *driver, bldc_startup_parameter_t *startup)
{
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 1);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], (uint32_t)(0));
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], (uint32_t)(0));
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], (uint32_t)(0));
    startup->charge_flag = 1;
}

/**
 * @brief 关闭所有下桥
 *
 * @param driver
 */
void all_mos_off(bldc_driver_t *driver)
{
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 1);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], (uint32_t)(0));
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], (uint32_t)(0));
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], (uint32_t)(0));
}

/**
 * @brief 转子位置获取
 *
 * @param driver
 * @param startup
 */
void align_pos_check_process(bldc_driver_t *driver, bldc_startup_parameter_t *startup, bldc_parameter_t *parameter)
{
    if (startup->adc_flag == 0 && startup->charge_flag == 0)
    {
        switch (startup->pos_check_stage)
        {
        case 0:
            // 电容充电
            capacitor_charge(driver, startup);
            startup->pos_check_stage = 10;
            break;
        case 10:
            // 注入第一个脉冲
            uv_w_phase_inject(driver, startup);
            startup->pos_check_stage = 1;
            break;
        case 1:
            // 电容充电
            capacitor_charge(driver, startup);
            startup->pos_check_stage = 20;
            break;
        case 20:
            // 注入第二个脉冲
            w_uv_phase_inject(driver, startup);
            startup->pos_check_stage = 2;
            break;
        case 2:
            // 电容充电
            capacitor_charge(driver, startup);
            startup->pos_check_stage = 30;
            break;
        case 30:
            wu_v_phase_inject(driver, startup);
            startup->pos_check_stage = 3;
            break;
        case 3:
            // 电容充电
            capacitor_charge(driver, startup);
            startup->pos_check_stage = 40;
            break;
        case 40:
            // 注入第四个脉冲
            v_wu_phase_inject(driver, startup);
            startup->pos_check_stage = 4;
            break;
        case 4:
            // 电容充电
            capacitor_charge(driver, startup);
            startup->pos_check_stage = 50;
            break;
        case 50:
            // 注入第五个脉冲
            vw_u_phase_inject(driver, startup);
            startup->pos_check_stage = 5;
            break;
        case 5:
            // 电容充电
            capacitor_charge(driver, startup);
            startup->pos_check_stage = 60;
            break;
        case 60:
            // 注入第六个脉冲
            u_vw_phase_inject(driver, startup);
            startup->pos_check_stage = 6;
            break;
        case 6:
            xSemaphoreGive(startup->info_sem); // 用户通知
            startup->align_flag = 1;
            if (startup->adc_check_buf[0] <= startup->adc_check_buf[1])
            {
                startup->phase_cnt |= 0x04;
            }
            if (startup->adc_check_buf[2] <= startup->adc_check_buf[3])
            {
                startup->phase_cnt |= 0x02;
            }
            if (startup->adc_check_buf[4] <= startup->adc_check_buf[5])
            {
                startup->phase_cnt |= 0x01;
            }
            parameter->pwm_duty = 200;
            startup->init_stage = startup->phase_cnt;
            all_mos_off(driver);
            switch (startup->phase_cnt)
            {
            case 5:
                parameter->phase_cnt = 1;
                bldc_turn(driver, parameter);
                break;
            case 1:
                parameter->phase_cnt = 2;
                bldc_turn(driver, parameter);

                break;
            case 3:
                parameter->phase_cnt = 3;
                bldc_turn(driver, parameter);

                break;
            case 2:
                parameter->phase_cnt = 4;
                bldc_turn(driver, parameter);

                break;
            case 6:
                parameter->phase_cnt = 5;
                bldc_turn(driver, parameter);

                break;
            case 4:
                parameter->phase_cnt = 6;
                bldc_turn(driver, parameter);

                break;
            default:
                bldc_turn(driver, parameter);
                break;
            }

            startup->pos_check_stage = 70;
        default:
            break;
        }
    }
}