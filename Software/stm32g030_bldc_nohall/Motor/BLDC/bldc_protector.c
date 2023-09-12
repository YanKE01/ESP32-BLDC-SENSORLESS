#include "bldc.h"


void judge_error_commutation(BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
                             BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                             BLDC_FLAGS_TypeDef *bldc_flags_ptr);

void fault_over_under_volt(BLDC_ADC_SAMPLE_TypeDef *bldc_adc_sample_ptr,
                           BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                           BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
                           BLDC_FLAGS_TypeDef *bldc_flags_ptr);

void fault_over_current(BLDC_ADC_SAMPLE_TypeDef *bldc_adc_sample_ptr,
                        BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                        BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
                        BLDC_FLAGS_TypeDef *bldc_flags_ptr);

void exit_fault_protect(BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
                        BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                        BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_param_ptr,
                        BLDC_FLAGS_TypeDef *bldc_flags_ptr,
                        PID_PARAMETERS_TypeDef *pid_param_ptr);


/**
 * @brief   故障检测函数，检测电流、电压、换相错误，以及退出故障保护的处理
 */

void fault_dection(BLDC_ADC_SAMPLE_TypeDef *bldc_adc_sample_ptr,
                   BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                   BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
                   BLDC_FLAGS_TypeDef *bldc_flags_ptr,
                   BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_param_ptr,
                   PID_PARAMETERS_TypeDef *pid_param_ptr)
{
    fault_over_under_volt(bldc_adc_sample_ptr, bldc_zc_param_ptr, bldc_param_ptr, bldc_flags_ptr);
    fault_over_current(bldc_adc_sample_ptr, bldc_zc_param_ptr, bldc_param_ptr, bldc_flags_ptr);
    // judge_error_commutation(bldc_param_ptr, bldc_zc_param_ptr, bldc_flags_ptr);
    exit_fault_protect(bldc_param_ptr, bldc_zc_param_ptr, bldc_startup_param_ptr, bldc_flags_ptr, pid_param_ptr);
}

/**
 * @brief   判断错误换相 
 */
void judge_error_commutation(BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
                             BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                             BLDC_FLAGS_TypeDef *bldc_flags_ptr)
{
    if(bldc_param_ptr->last_phase == bldc_param_ptr->phase_cnt)
    {
        if(++bldc_zc_param_ptr->phase_err_cnt >= 1000)
        {
            bldc_zc_param_ptr->phase_err_cnt = 0;
            bldc_zc_param_ptr->stop_time = BLDC_DELAY_TIME;
            bldc_stop(bldc_param_ptr, bldc_flags_ptr);
            bldc_param_ptr->faults = OVERTIME_PHASE_CHANGE_6;
        }
    }else
    {
        bldc_zc_param_ptr->phase_err_cnt = 0;
    }
    bldc_param_ptr->last_phase = bldc_param_ptr->phase_cnt;
}

/**
 * @brief   过压或欠压保护函数
 */
void fault_over_under_volt(BLDC_ADC_SAMPLE_TypeDef *bldc_adc_sample_ptr,
                           BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                           BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
                           BLDC_FLAGS_TypeDef *bldc_flags_ptr)
{
    static uint16_t volt_protect_cnt = 0;
    if(bldc_adc_sample_ptr->bus_volt<MIN_BUS_VOLT || bldc_adc_sample_ptr->bus_volt>MAX_BUS_VOLT)
    {
        if(++volt_protect_cnt >= 1000)
        {
            /* 过压或欠压一定时间后，电机停转，更新电机故障状态 */
            volt_protect_cnt = 0;
            bldc_zc_param_ptr->stop_time = BLDC_DELAY_TIME;
            bldc_stop(bldc_param_ptr, bldc_flags_ptr);
            bldc_param_ptr->faults = OVER_UNDER_VOLT_1;
        }
    }else
    {
        volt_protect_cnt = 0;
    }
}

/**
 * @brief   过流保护函数 
 */
void fault_over_current(BLDC_ADC_SAMPLE_TypeDef *bldc_adc_sample_ptr,
                        BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                        BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
                        BLDC_FLAGS_TypeDef *bldc_flags_ptr)
{
    static uint16_t current_protect_cnt = 0;
    if(bldc_adc_sample_ptr->bus_current > MAX_BUS_CURRENT)
    {
        if(++current_protect_cnt >=1000)
        {
            current_protect_cnt = 0;
            bldc_zc_param_ptr->stop_time = BLDC_DELAY_TIME;
            bldc_stop(bldc_param_ptr, bldc_flags_ptr);
            bldc_param_ptr->faults = OVER_CURRENT_2;
        }
    }else
    {
        current_protect_cnt = 0;
    }
}



/**
 * @brief   保护一段时间之后退出保护
 */

void exit_fault_protect(BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
                        BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                        BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_param_ptr,
                        BLDC_FLAGS_TypeDef *bldc_flags_ptr,
                        PID_PARAMETERS_TypeDef *pid_param_ptr)
{
    static uint16_t exit_fault_protect_cnt = 0;
    if(bldc_param_ptr->status == BLDC_STOP_5)
    {
        /* bldc处于故障状态 */
        if(bldc_param_ptr->faults != RUNNORMAL_0)
        {
            if(++exit_fault_protect_cnt >= bldc_zc_param_ptr->stop_time)
            {
                /* 退出保护 */
                exit_fault_protect_cnt = 0;
                /* 初始化BLDC相关参数 */
                bldc_init(bldc_param_ptr, bldc_startup_param_ptr, bldc_zc_param_ptr, pid_param_ptr, bldc_flags_ptr);
                /* 使能换相和计时的定时器TIM14 TIM16 */
                __HAL_TIM_ENABLE(&htim14);
                __HAL_TIM_ENABLE(&htim16);
            }
        }else
        {
            exit_fault_protect_cnt = 0;
        }
    }else{}
}
