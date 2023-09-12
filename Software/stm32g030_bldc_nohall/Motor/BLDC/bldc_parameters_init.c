#include "bldc.h"

BLDC_PARAMETERS_TypeDef bldc_parameters = {0};
BLDC_ZC_PARAMETERS_TypeDef bldc_zc_parameters = {0};
BLDC_FLAGS_TypeDef bldc_flags = {0};
BLDC_STARTUP_PARAMETERS_TypedDef bldc_startup_parameters = {0};
BLDC_PROTECTOR_TypeDef bldc_protector = {0};
BLDC_ADC_SAMPLE_TypeDef bldc_adc_sample = {0};
PID_PARAMETERS_TypeDef pid_speed = {0};

void bldc_parameters_init(BLDC_PARAMETERS_TypeDef *bldc_param_ptr)
{
    bldc_param_ptr->control_mode = Control_Mode;
    bldc_param_ptr->status = BLDC_INIT_0;
    bldc_param_ptr->faults = RUNNORMAL_0;
    bldc_param_ptr->phase_cnt = 0;
    bldc_param_ptr->last_phase = 0;
    bldc_param_ptr->next_phase = 0;
    bldc_param_ptr->actual_speed = 0;
    bldc_param_ptr->user_require_speed = 0;
    bldc_param_ptr->theory_speed = 0;
    bldc_param_ptr->duty = 0;
    bldc_param_ptr->step_cnt = 0;
    bldc_param_ptr->bemf = 0;
    bldc_param_ptr->last_speed = 0;
}

void bldc_zc_parameters_init(BLDC_ZC_PARAMETERS_TypeDef * bldc_zc_param_ptr)
{
    bldc_zc_param_ptr->speed_time_cnt = 0;
    bldc_zc_param_ptr->speed_time = 0;
    bldc_zc_param_ptr->speed_time_tmp = 0;
    bldc_zc_param_ptr->speed_time_sum = 0;
    bldc_zc_param_ptr->stop_time = 0;
    bldc_zc_param_ptr->mask_time = Low_DutyMask_Time;
    bldc_zc_param_ptr->delay_time30 = 0;
}

void bldc_flags_init(BLDC_FLAGS_TypeDef *bldc_flags_ptr)
{
    bldc_flags_ptr->sys_start = 1;
    bldc_flags_ptr->bldc_stop = 0;
    bldc_flags_ptr->angle_mask = 0;
    bldc_flags_ptr->change_phase = 0;
    bldc_flags_ptr->flag_speed_time = 0;
} 

void bldc_startup_parameters_init(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_param_ptr)
{
    bldc_startup_param_ptr->flag_charge = 0;
    bldc_startup_param_ptr->flag_adc = 0;
    bldc_startup_param_ptr->pos_idx = 0;
    bldc_startup_param_ptr->pos_check_stage = 0;
    bldc_startup_param_ptr->phase_cnt = 0;
    bldc_startup_param_ptr->initial_stage = 0;
    bldc_startup_param_ptr->charge_cnt = 0;
}


void pid_speed_init(PID_PARAMETERS_TypeDef *pid_speed_ptr)
{
    pid_speed_ptr->kp = 0.01f;
    pid_speed_ptr->ki = 0.005f;
    pid_speed_ptr->kd = 0.0f;
    pid_speed_ptr->max_value = PWM_DUTY_MAX;
    pid_speed_ptr->min_value = PWM_DUTY_MIN;
    pid_speed_ptr->error = 0.0f;
    pid_speed_ptr->fpall_err = 0.0f;
    pid_speed_ptr->out = 0;
    pid_speed_ptr->purpose = INIT_PURPOSE;      // 重启PID时，重新计算错误积分
    pid_speed_ptr->ui = 0.0f;
    pid_speed_ptr->max = PWM_DUTY_MAX;    
    pid_speed_ptr->min = PWM_DUTY_MIN;
}
