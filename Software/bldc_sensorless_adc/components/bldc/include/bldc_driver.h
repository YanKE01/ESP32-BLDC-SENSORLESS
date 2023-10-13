#pragma once

#include "esp_err.h"
#include "bldc.h"

void bldc_adc_init(bldc_adc_t *bldc_adc);
esp_err_t bldc_mcpwm_init(bldc_driver_t *driver, bldc_startup_parameter_t *startup, bldc_adc_t *adc, bldc_parameter_t *parameter);
void bldc_u_phase_test(bldc_driver_t *driver,bldc_adc_t *adc);
void bldc_turn(bldc_driver_t *driver, bldc_parameter_t *parameter);
