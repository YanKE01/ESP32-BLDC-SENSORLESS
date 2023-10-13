#include "bldc_common.h"

void uh_vl(bldc_driver_t *driver, bldc_parameter_t *parameter)
{
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], parameter->pwm_duty);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 1);
}

void uh_wl(bldc_driver_t *driver, bldc_parameter_t *parameter)
{
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], parameter->pwm_duty);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 0);
}

void vh_ul(bldc_driver_t *driver, bldc_parameter_t *parameter)
{
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], parameter->pwm_duty);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 1);
}

void vh_wl(bldc_driver_t *driver, bldc_parameter_t *parameter)
{
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], parameter->pwm_duty);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 0);
}

void wh_ul(bldc_driver_t *driver, bldc_parameter_t *parameter)
{
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], parameter->pwm_duty);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 1);
}

void wh_vl(bldc_driver_t *driver, bldc_parameter_t *parameter)
{
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[u_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[v_phase], 0);
    mcpwm_comparator_set_compare_value(driver->mcpwm.comparators[w_phase], parameter->pwm_duty);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[u_phase], 1);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[v_phase], 0);
    gpio_set_level(driver->lin_gpio.lin_gpio_nums[w_phase], 1);
}