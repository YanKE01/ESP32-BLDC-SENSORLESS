#include "bldc.h"


/**
 * @brief   BLDC的PID控制计算
 */
void bldc_pid_cacl(int32_t ref_speed, int32_t actual_speed, PID_PARAMETERS_TypeDef *pid_param_ptr, BLDC_PARAMETERS_TypeDef *bldc_param_ptr)
{
    float tmp;  // 临时变量
    if(pid_param_ptr->purpose == INIT_PURPOSE)
    {
        pid_param_ptr->purpose = RUN_PURPOSE;
        pid_param_ptr->fpall_err = 0;                   // 清空累计误差
    }
    pid_param_ptr->error = ref_speed - actual_speed;    // 计算误差
    pid_param_ptr->fpall_err += pid_param_ptr->error * pid_param_ptr->kp;
    pid_param_ptr->ui = pid_param_ptr->ki * pid_param_ptr->fpall_err;
    if(pid_param_ptr->fpall_err > pid_param_ptr->max_value/pid_param_ptr->ki)
    {
        pid_param_ptr->fpall_err = pid_param_ptr->max_value/pid_param_ptr->ki;  // 控制积分上限
    }
    if(pid_param_ptr->fpall_err < pid_param_ptr->min_value/pid_param_ptr->ki)
    {
        pid_param_ptr->fpall_err = pid_param_ptr->min_value/pid_param_ptr->ki;  // 控制积分下限
    }
    tmp = pid_param_ptr->kp * pid_param_ptr->error + pid_param_ptr->ui;
    pid_param_ptr->out = tmp;
    /* 控制积分的上下限 */
    if(pid_param_ptr->out > pid_param_ptr->max)
    {
        pid_param_ptr->out = pid_param_ptr->max;
    }
    if(pid_param_ptr->out < pid_param_ptr->min)
    {
        pid_param_ptr->out = pid_param_ptr->min;
    }
    bldc_param_ptr->duty = pid_param_ptr->out;
}
