#pragma once

#include "bldc.h"

void uh_vl(bldc_driver_t *driver, bldc_parameter_t *parameter);
void uh_wl(bldc_driver_t *driver, bldc_parameter_t *parameter);
void vh_ul(bldc_driver_t *driver, bldc_parameter_t *parameter);
void vh_wl(bldc_driver_t *driver, bldc_parameter_t *parameter);
void wh_ul(bldc_driver_t *driver, bldc_parameter_t *parameter);
void wh_vl(bldc_driver_t *driver, bldc_parameter_t *parameter);
