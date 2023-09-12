#include "bldc.h"

/**
 * @brief   计算时间 60°
 */
void calc_speed_time(BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                     BLDC_FLAGS_TypeDef *bldc_flags_ptr)
{
    /* TIM16的计数值更新中断函数中，进行换相操作 */
    __HAL_TIM_SetAutoreload(&htim16, bldc_zc_param_ptr->delay_time30);      // 设置TIM的ARR寄存器的值为换相延时时间
    __HAL_TIM_SetCounter(&htim16, 0);                                       // 清空计数值
    __HAL_TIM_ENABLE(&htim16);                                              // 使能定时器
    bldc_zc_param_ptr->speed_time_tmp = __HAL_TIM_GetCounter(&htim14);      // 过零点时间间隔
    __HAL_TIM_SetCounter(&htim14, 0);
    bldc_zc_param_ptr->delay_time30 = (uint16_t)(bldc_zc_param_ptr->speed_time_tmp/2);      // 获得换相延时时间
    bldc_zc_param_ptr->speed_time = bldc_zc_param_ptr->speed_time_tmp;
    bldc_flags_ptr->change_phase = 1;   // 开始换相
}

/**
 * @brief   BLDC速度控制
 */
void bldc_speed_ctrl(BLDC_PARAMETERS_TypeDef *bldc_param_ptr, BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr, PID_PARAMETERS_TypeDef *pid_param_ptr)
{
    if(bldc_param_ptr->control_mode == CLOSED_SPEEDLOOP_Halless)        // 速度闭环
    {
        bldc_param_ptr->last_speed = SPEED_FACTOR / bldc_zc_param_ptr->speed_time;
        FIRST_ORDER_LFP_CACL(bldc_param_ptr->last_speed, bldc_param_ptr->actual_speed, 0.06);   // 对转速进行一阶滤波
        bldc_pid_cacl(bldc_param_ptr->user_require_speed, bldc_param_ptr->actual_speed, pid_param_ptr, bldc_param_ptr);
    }else if(bldc_param_ptr->control_mode == OPEN_LOOP_Halless)         // 开环运行
    {
        bldc_param_ptr->step_cnt ++;
        bldc_param_ptr->last_speed = SPEED_FACTOR / bldc_zc_param_ptr->speed_time;
        FIRST_ORDER_LFP_CACL(bldc_param_ptr->last_speed, bldc_param_ptr->actual_speed, 0.06);   // 段转速进行一阶滤波
        if(bldc_param_ptr->duty < PWM_DUTY_MIN)
        {   
            if(bldc_param_ptr->step_cnt > LOW_DUTY_COUNT)
            {
                bldc_param_ptr->step_cnt = 0;
                bldc_param_ptr->duty += ADD_DUTY1;
            }
        }else
        {
            if(bldc_param_ptr->duty < bldc_param_ptr->user_require_speed)
            {
                if(bldc_param_ptr->duty <= DUTYTHRESHOLD1)
                {
                    if(bldc_param_ptr->step_cnt > LOW_DUTY_COUNT)
                    {
                        bldc_param_ptr->step_cnt = 0;
                        bldc_param_ptr->duty += ADD_DUTY1;
                    }
                }else if(bldc_param_ptr->duty>DUTYTHRESHOLD2 && bldc_param_ptr->duty<DUTYTHRESHOLD3)
                {
                    if(bldc_param_ptr->step_cnt > HIGH_DUTY_COUNT)
                    {
                        bldc_param_ptr->step_cnt = 0;
                        bldc_param_ptr->duty += ADD_DUTY2;
                    }
                }else
                {
                    if(bldc_param_ptr->step_cnt > HIGH_DUTY_COUNT)
                    {
                        bldc_param_ptr->step_cnt = 0;
                        bldc_param_ptr->duty += ADD_DUTY3;
                    }
                }
            }else
            {
                bldc_param_ptr->step_cnt = 0;
                bldc_param_ptr->duty -= ADD_DUTY2;
            }
            /* 限制输出占空比的最大最小值 */
            if(bldc_param_ptr->duty <= PWM_DUTY_MIN)
            {
                bldc_param_ptr->duty = PWM_DUTY_MIN;
            }else if(bldc_param_ptr->duty >= PWM_DUTY_MAX)
            {
                bldc_param_ptr->duty = PWM_DUTY_MAX;
            }
        }
    }else{}
}

/**
 * @brief   对转速时间求平均
 */
void calc_avg_speed_time(BLDC_FLAGS_TypeDef *bldc_flags_ptr, BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr)
{
    if(!bldc_flags_ptr->flag_speed_time)
    {
        bldc_zc_param_ptr->speed_time = bldc_zc_param_ptr->speed_time_tmp;
    }
    bldc_zc_param_ptr->speed_time_sum += bldc_zc_param_ptr->speed_time_tmp;
    if(++bldc_zc_param_ptr->speed_time_cnt >= 6)
    {
        if(!bldc_flags_ptr->flag_speed_time)
        {
            bldc_flags_ptr->flag_speed_time = 1;
        }
        bldc_zc_param_ptr->speed_time = (uint16_t)(bldc_zc_param_ptr->speed_time_sum/6);
        bldc_zc_param_ptr->speed_time_sum = 0;
        bldc_zc_param_ptr->speed_time_cnt = 0;
    }
}


