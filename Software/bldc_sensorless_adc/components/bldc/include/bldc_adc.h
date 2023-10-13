#pragma once
#include "bldc.h"

void bldc_adc_init(bldc_adc_t *bldc_adc);
int bldc_adc_read_amp(bldc_adc_t *bldc_adc);
int bldc_adc_read_vbus(bldc_adc_t *bldc_adc);