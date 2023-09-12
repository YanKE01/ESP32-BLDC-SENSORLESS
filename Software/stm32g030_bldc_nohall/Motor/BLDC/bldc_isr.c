#include "bldc.h"


/**
 * @brief   定时器的周期回调函数
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    
    /* 在TIM16的定时器中断函数中，完成换相动作 */
    if(htim == &htim16)
    {
        /* 换相屏蔽时，不进行换相 */
        if(bldc_flags.angle_mask == 1)
        {
            bldc_flags.angle_mask = 0;
            __HAL_TIM_DISABLE(&htim16);     // 失能定时器
        }
        /* 换相 */
        if(bldc_flags.change_phase == 1)
        {
            bldc_flags.change_phase = 0;
            bldc_turn(&bldc_parameters, &bldc_flags);
            __HAL_TIM_SET_AUTORELOAD(&htim16, bldc_zc_parameters.mask_time);        // 设置TIM的ARR寄存器值为换相屏蔽时间，换相屏蔽时间后定时器中断将会被再次触发
            __HAL_TIM_SET_COUNTER(&htim16, 0);
            bldc_flags.angle_mask = 1;
        }
    }
}

/**
 * @brief   DMA的回调函数，处理过零点检测 16KHz触发频率
 */
void DMA1_Channel1_XferCpltCallback(struct __DMA_HandleTypeDef *hdma)
{   
    if(hdma == &hdma_adc1)
    {   
        switch(bldc_parameters.status)
        {
            case BLDC_ALIGNMENT_1:      // 转子位置检测，并对齐
            {
                align_pos_check_process(&bldc_startup_parameters, 
                                        &bldc_protector,
                                        &bldc_parameters,
                                        &bldc_flags,
                                        &bldc_zc_parameters);
                /* 充电标志位或电流检测标志位置1，则充电时间计数器计数 */
                if(bldc_startup_parameters.flag_adc==1 || bldc_startup_parameters.flag_charge==1)
                {
                    bldc_startup_parameters.charge_cnt ++;
                }   
                /* 电流检测标志位置1，则获取脉冲注入下的响应电流 */
                if(bldc_startup_parameters.flag_adc == 1)
                {
                    if(bldc_startup_parameters.pos_idx <= 5)    // 检测序号，防止出错
                    {
                        bldc_startup_parameters.adc_check_buf[bldc_startup_parameters.pos_idx] = adc1_value[1];
                    }   
                }
                if(bldc_startup_parameters.flag_charge == 1)    // 充电标志位置1
                {
                    /* 充电时间到 */
                    if(bldc_startup_parameters.charge_cnt >= LONG_PULSE_CNT)
                    {
                        bldc_startup_parameters.charge_cnt = 0;     // 充电计数清0
                        all_mos_off();      // 关闭六个MOS管
                        bldc_startup_parameters.flag_charge = 0;    // 充电标志位清0
                    }
                }else if(bldc_startup_parameters.flag_adc == 1)
                {
                    /* 电流检测时间到 */
                    if(bldc_startup_parameters.charge_cnt >= SHORT_PULSE_CNT)
                    {
                        bldc_startup_parameters.charge_cnt  = 0;    // 电流检测时间清0 
                        all_mos_off();      // 关闭六个MOS管
                        bldc_startup_parameters.flag_adc = 0;    // 充电标志位清0
                    }
                }else{}
            }break;
            case BLDC_DRAG_2:
            {
                startup_drag(&bldc_parameters, &bldc_zc_parameters, &bldc_flags);
            }break;
            case BLDC_RUN_3:
            {
                bemf_check(&bldc_flags, &bldc_parameters, &bldc_zc_parameters);
            }break;
            default: break;
        }
    }
}

/**
 * @brief   在SYSTICK中断中实现BLDC的速度环控制
 */
void HAL_IncTick(void)
{
    adc_value_transfer(&bldc_adc_sample, &bldc_parameters);     // 转换ADC值
    calc_avg_speed_time(&bldc_flags, &bldc_zc_parameters);      // 计算平均转速
    bldc_ctrl(&bldc_parameters, &bldc_flags, &bldc_startup_parameters, &bldc_zc_parameters, &pid_speed);
    fault_dection(&bldc_adc_sample, &bldc_zc_parameters, &bldc_parameters, &bldc_flags, &bldc_startup_parameters, &pid_speed);
}

