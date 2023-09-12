#include "bldc.h"


void uv_w_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr);
void w_uv_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr);
void wu_v_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr);
void v_wu_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr);
void vw_u_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr);
void u_vw_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr);
void capacitor_charge(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr);
void all_mos_off(void);
void bldc_alignment(BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr, 
                    BLDC_FLAGS_TypeDef *bldc_flags_ptr, 
                    BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_parameters_ptr);
void enter_drag_init(BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr, 
                    BLDC_ZC_PARAMETERS_TypeDef* bldc_zc_parameters_ptr);
void enter_run_init(BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_parameters_ptr,
                    BLDC_FLAGS_TypeDef *bldc_flags_ptr,
                    BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr);


/**
 * @brief   UV_W注入脉冲
 */
void uv_w_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr)
{
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, LOCK_DUTY);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, LOCK_DUTY);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_RESET);
    bldc_startup_parameters_ptr->flag_adc = 1;
    bldc_startup_parameters_ptr->pos_idx = 0;
}

/**
 * @brief   W_UV注入脉冲
 */
void w_uv_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr)
{
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, LOCK_DUTY);
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_RESET);
    bldc_startup_parameters_ptr->flag_adc = 1;
    bldc_startup_parameters_ptr->pos_idx = 1;
}

/**
 * @brief   WU_V注入脉冲
 */
void wu_v_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr)
{
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, LOCK_DUTY);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, LOCK_DUTY);
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_RESET);
    bldc_startup_parameters_ptr->flag_adc = 1;
    bldc_startup_parameters_ptr->pos_idx = 2;
}

/**
 * @brief   V_WU注入脉冲
 */
void v_wu_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr)
{
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, LOCK_DUTY);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_RESET);
    bldc_startup_parameters_ptr->flag_adc = 1;
    bldc_startup_parameters_ptr->pos_idx = 3;
}

/**
 * @brief   VW_U注入脉冲
 */
void vw_u_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr)
{
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, LOCK_DUTY);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, LOCK_DUTY);
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_RESET);
    bldc_startup_parameters_ptr->flag_adc = 1;
    bldc_startup_parameters_ptr->pos_idx = 4;
}

/**
 * @brief   U_VW注入脉冲
 */
void u_vw_phase_inject(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr)
{
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, LOCK_DUTY);
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_RESET);
    bldc_startup_parameters_ptr->flag_adc = 1;
    bldc_startup_parameters_ptr->pos_idx = 5;
}

/**
 * @brief   电容充电
 */
void capacitor_charge(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr)
{
    /* 关上桥 */
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 0);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, 0);
    /* 开下桥 */
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_SET);
    /* 置位充电标志位 */
    bldc_startup_parameters_ptr->flag_charge = 1;
}

/**
 * @brief   关闭所有mos管
 */
void all_mos_off(void)
{
    /* 关上桥 */
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 0);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, 0);
    /* 关下桥 */
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_RESET);
}
/**
 * @brief   对齐定位
 */
void bldc_alignment(BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr, 
                    BLDC_FLAGS_TypeDef *bldc_flags_ptr, 
                    BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_parameters_ptr)
{
    bldc_parameters_ptr->duty = ALIGNMENTDUTY;          // 定位对齐占空比
    bldc_parameters_ptr->phase_cnt ++;                  // 0-1
    bldc_turn(bldc_parameters_ptr, bldc_flags_ptr);     // 强制换相
    // HAL_Delay(ALIGNMENTNMS);                            // 对齐时间
    enter_drag_init(bldc_parameters_ptr, bldc_zc_parameters_ptr);   // 进入强拖加速前，对相关参数进行初始化操作
}   
/**
 * @brief   进入强拖前，对相关参数进行初始化操作
 */
void enter_drag_init(BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr, 
                    BLDC_ZC_PARAMETERS_TypeDef* bldc_zc_parameters_ptr)
{
    bldc_zc_parameters_ptr->adc_time_cnt = 0;
    bldc_zc_parameters_ptr->drag_time = RAMP_TIM_STA;
    bldc_parameters_ptr->status = BLDC_DRAG_2;
    HAL_TIM_Base_Start(&htim14);
}

/**
 * @brief   强拖使转速上升后，进入过零检测前，对相关参数进行初始化操作
 */
void enter_run_init(BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_parameters_ptr,
                    BLDC_FLAGS_TypeDef *bldc_flags_ptr,
                    BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr)
{
    bldc_zc_parameters_ptr->adc_time_cnt = 0;
    bldc_zc_parameters_ptr->timer_30ut_cnt = 0;
    bldc_zc_parameters_ptr->blanking_cnt = 0;
    bldc_flags_ptr->change_phase = 0;
    bldc_flags_ptr->angle_mask = 0;
    bldc_parameters_ptr->status = BLDC_RUN_3;
}

/**
 * @brief   强拖加速
 */
void startup_drag(BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr,
                  BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_parameters_ptr,
                  BLDC_FLAGS_TypeDef *bldc_flags_ptr)
{
    if(++bldc_zc_parameters_ptr->adc_time_cnt >= bldc_zc_parameters_ptr->drag_time)
    {
        bldc_zc_parameters_ptr->adc_time_cnt = 0;
        bldc_parameters_ptr->duty += RAMP_DUTY_INC;
        bldc_zc_parameters_ptr->drag_time -= (bldc_zc_parameters_ptr->drag_time / RAMP_TIM_STEP) + 1;
        if(bldc_zc_parameters_ptr->drag_time < RAMP_TIM_END)
        {
            bldc_zc_parameters_ptr->drag_time = RAMP_TIM_END;
            enter_run_init(bldc_zc_parameters_ptr, bldc_flags_ptr, bldc_parameters_ptr);
            return;
        }
        bldc_parameters_ptr->phase_cnt ++;
        bldc_zc_parameters_ptr->delay_time30 = __HAL_TIM_GetCounter(&htim14);   // 过零点时间间隔
        bldc_turn(&bldc_parameters, &bldc_flags);       // 换相
        __HAL_TIM_SetCounter(&htim14, 0);
        bldc_zc_parameters_ptr->delay_time30 = bldc_zc_parameters_ptr->delay_time30/2;
    }
    /* 限制输出占空比的最大最小值 */
    if(bldc_parameters_ptr->duty < RAMP_DUTY_STA)
    {
        bldc_parameters_ptr->duty = RAMP_DUTY_STA;
    }else if(bldc_parameters_ptr->duty > RAMP_DUTY_END)
    {
        bldc_parameters_ptr->duty = RAMP_DUTY_END;
    }
}

/**
 * @brief   BLDC启动时的转子位置检测
 */
void align_pos_check_process(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr,
                             BLDC_PROTECTOR_TypeDef *bldc_protector_ptr,
                             BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr,
                             BLDC_FLAGS_TypeDef *bldc_flags_ptr,
                             BLDC_ZC_PARAMETERS_TypeDef * bldc_zc_parameters_ptr)
{   
    /* 脉冲注入下，采集总线电流 */
    if(bldc_startup_parameters_ptr->flag_adc==0 && bldc_startup_parameters_ptr->flag_charge==0)
    {
        switch(bldc_startup_parameters_ptr->pos_check_stage)
        {
            case 0:     // 电容充电
            {
                capacitor_charge(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 10;
            }break;
            case 10:    // 注入第一个脉冲
            {
                uv_w_phase_inject(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 1;
            }break;
            case 1:     // 电容充电
            {
                capacitor_charge(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 20;
            }break;
            case 20:    // 注入第二个脉冲
            {
                w_uv_phase_inject(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 2;
            }break;
            case 2:     // 电容充电
            {
                capacitor_charge(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 30;
            }break;
            case 30:    // 注入第三个脉冲
            {
                wu_v_phase_inject(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 3;
            }break;
            case 3:     // 电容充电
            {   
                capacitor_charge(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 40;
            }break;
            case 40:    // 注入第四个脉冲
            {
                v_wu_phase_inject(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 4;
            }break; 
            case 4:     // 电容充电
            {
                capacitor_charge(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 50;
            }break;
            case 50:    // 注入第五个脉冲
            {
                vw_u_phase_inject(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 5;
            }break; 
            case 5:     // 电容充电
            {
                capacitor_charge(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 60;
            }break;
            case 60:    // 注入第六个脉冲
            {
                u_vw_phase_inject(bldc_startup_parameters_ptr);
                bldc_startup_parameters_ptr->pos_check_stage = 6;
            }break;
            case 6:     // 
            {
                bldc_startup_parameters_ptr->phase_cnt = 0;
                /* 比较脉冲激励下的总线电流，获取转子位置 */
                if(bldc_startup_parameters_ptr->adc_check_buf[0]<=bldc_startup_parameters_ptr->adc_check_buf[1])
                    bldc_startup_parameters_ptr->phase_cnt |= 0x04;
                if(bldc_startup_parameters_ptr->adc_check_buf[2]<=bldc_startup_parameters_ptr->adc_check_buf[3])
                    bldc_startup_parameters_ptr->phase_cnt |= 0x02;
                if(bldc_startup_parameters_ptr->adc_check_buf[4]<=bldc_startup_parameters_ptr->adc_check_buf[5])
                    bldc_startup_parameters_ptr->phase_cnt |= 0x01;
                bldc_startup_parameters_ptr->initial_stage = bldc_startup_parameters_ptr->phase_cnt;
                bldc_protector_ptr->flag_hw_over_current = 1;           // 脉冲注入之后，才能开启硬件过流保护
                all_mos_off();      // 关闭六个MOS管
                bldc_parameters_ptr->duty = ALIGNMENTDUTY;
                switch (bldc_startup_parameters_ptr->phase_cnt)
                {
                    case 5:
                    {
                        bldc_parameters_ptr->phase_cnt = 1;
                        bldc_turn(bldc_parameters_ptr, bldc_flags_ptr);     // 强制换相
                        // HAL_Delay(ALIGNMENTNMS);                            // 对齐时间
                        enter_drag_init(bldc_parameters_ptr, bldc_zc_parameters_ptr);       // 准备进入强拖
                    }break;
                    case 1:
                    {
                        bldc_parameters_ptr->phase_cnt = 2;
                        bldc_turn(bldc_parameters_ptr, bldc_flags_ptr);     // 强制换相
                        // HAL_Delay(ALIGNMENTNMS);                            // 对齐时间
                        enter_drag_init(bldc_parameters_ptr, bldc_zc_parameters_ptr);       // 准备进入强拖
                    }break;
                    case 3:
                    {
                        bldc_parameters_ptr->phase_cnt = 3;
                        bldc_turn(bldc_parameters_ptr, bldc_flags_ptr);     // 强制换相
                        // HAL_Delay(ALIGNMENTNMS);                            // 对齐时间
                        enter_drag_init(bldc_parameters_ptr, bldc_zc_parameters_ptr);       // 准备进入强拖
                    }break;
                    case 2:
                    {
                        bldc_parameters_ptr->phase_cnt = 4;
                        bldc_turn(bldc_parameters_ptr, bldc_flags_ptr);     // 强制换相
                        // HAL_Delay(ALIGNMENTNMS);                            // 对齐时间
                        enter_drag_init(bldc_parameters_ptr, bldc_zc_parameters_ptr);       // 准备进入强拖
                    }break;
                    case 6:
                    {
                        bldc_parameters_ptr->phase_cnt = 5;
                        bldc_turn(bldc_parameters_ptr, bldc_flags_ptr);     // 强制换相
                        // HAL_Delay(ALIGNMENTNMS);                            // 对齐时间
                        enter_drag_init(bldc_parameters_ptr, bldc_zc_parameters_ptr);       // 准备进入强拖
                    }break;
                    case 4:
                    {
                        bldc_parameters_ptr->phase_cnt = 6;
                        bldc_turn(bldc_parameters_ptr, bldc_flags_ptr);     // 强制换相
                        // HAL_Delay(ALIGNMENTNMS);                            // 对齐时间
                        enter_drag_init(bldc_parameters_ptr, bldc_zc_parameters_ptr);       // 准备进入强拖
                    }break;
                    default:
                    {
                        bldc_turn(bldc_parameters_ptr, bldc_flags_ptr);     // 强制换相
                        // HAL_Delay(ALIGNMENTNMS);
                        bldc_stop(bldc_parameters_ptr, bldc_flags_ptr);
                    }break;
                }
                bldc_startup_parameters_ptr->pos_check_stage = 70;
            }break;
            default: break;
        }
    }
}
