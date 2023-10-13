#pragma once
#include "bldc.h"
#include "stdlib.h"

void all_mos_off(bldc_driver_t *driver);
void align_pos_check_process(bldc_driver_t *driver, bldc_startup_parameter_t *startup,bldc_parameter_t *parameter);
void uv_w_phase_inject(bldc_driver_t *driver, bldc_startup_parameter_t *startup);