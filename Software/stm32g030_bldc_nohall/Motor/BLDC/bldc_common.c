#include "bldc.h"

void uh_vl(void);
void uh_wl(void);
void vh_ul(void);
void vh_wl(void);
void wh_ul(void);
void wh_vl(void);

/**
 * @brief U相上桥壁和V相下桥臂导通
 */
static void uh_vl(void)
{
    htim3.Instance->CCR1 = 0;
    htim3.Instance->CCR2 = 0;
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_RESET);
    htim3.Instance->CCR3 = bldc_parameters.duty;
}

/**
 * @brief U相上桥壁和W相下桥臂导通
 */
static void uh_wl(void)
{
    htim3.Instance->CCR1 = 0;
    htim3.Instance->CCR2 = 0;
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_RESET);
    htim3.Instance->CCR3 = bldc_parameters.duty;
}

/**
 * @brief V相上桥壁和U相下桥臂导通
 */
static void vh_ul(void)
{
    htim3.Instance->CCR1 = 0;
    htim3.Instance->CCR3 = 0;
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_RESET);
    htim3.Instance->CCR2 = bldc_parameters.duty;
}

/**
 * @brief V相上桥壁和W相下桥臂导通
 */
static void vh_wl(void)
{
    htim3.Instance->CCR1 = 0;
    htim3.Instance->CCR3 = 0;
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_RESET);
    htim3.Instance->CCR2 = bldc_parameters.duty;
}

/**
 * @brief W相上桥壁和U相下桥臂导通
 */
static void wh_ul(void)
{
    htim3.Instance->CCR2 = 0;
    htim3.Instance->CCR3 = 0;
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_RESET);
    htim3.Instance->CCR1 = bldc_parameters.duty;
}

/**
 * @brief W相上桥壁和V相下桥臂导通
 */
static void wh_vl(void)
{
    htim3.Instance->CCR2 = 0;
    htim3.Instance->CCR3 = 0;
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_RESET);
    htim3.Instance->CCR1 = bldc_parameters.duty;
}

/**
 * @brief   BLDC初始化
 */
void bldc_init(BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
               BLDC_STARTUP_PARAMETERS_TypedDef* bldc_startup_param_ptr,
               BLDC_ZC_PARAMETERS_TypeDef* bldc_zc_param_ptr,
               PID_PARAMETERS_TypeDef *pid_param_ptr,
               BLDC_FLAGS_TypeDef *bldc_flags_ptr)
{
    bldc_flags_init(bldc_flags_ptr);
    bldc_parameters_init(bldc_param_ptr);
    bldc_startup_parameters_init(bldc_startup_param_ptr);
    bldc_zc_parameters_init(bldc_zc_param_ptr);
    pid_speed_init(pid_param_ptr);
    /* 使能上管 */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    /* 下管截止 */
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_SET);
    bldc_param_ptr->status = BLDC_ALIGNMENT_1;
    // bldc_param_ptr->faults = RUNNORMAL_0;
}


/**
 * @brief   BLDC开机 即关闭下MOS管，使能上MOS的PWM输出
 */
void bldc_start(void)
{
    HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_SET);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
}

/**
 * @brief   BLDC停机
 * @param   BLDC_PARAMETERS_TypeDef* bldc参数指针;
 *          BLDC_FLAGS_TypeDef * bldc标志位指针
 */
void bldc_stop(BLDC_PARAMETERS_TypeDef *bldc_param_ptr, BLDC_FLAGS_TypeDef *bldc_flags_ptr)
{
    bldc_param_ptr->status = BLDC_STOP_5;
    bldc_param_ptr->duty = 0;
    bldc_param_ptr->phase_cnt = 0;
    /* UVW相 PWM清0 */
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 0);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, 0);
    /* 关闭PWM通道 */
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
    __HAL_TIM_DISABLE(&htim14);             // 失能定时器
    __HAL_TIM_DISABLE(&htim16);
    __HAL_TIM_SetCounter(&htim14, 0);       // 计数清0
    __HAL_TIM_SetCounter(&htim16, 0);
    /* 强刹车 */
    #if(STOPMODE == BRAKEDOWN)
        HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_RESET);
        HAL_Delay(300);
        bldc_flags_ptr->bldc_stop = 1;
    #endif
    /* 自由刹车 */
    #if(STOPMODE == FREEDOWN)
        HAL_GPIO_WritePin(U_L_GPIO_PORT, U_L_GPIO_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(V_L_GPIO_PORT, V_L_GPIO_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(W_L_GPIO_PORT, W_L_GPIO_PIN, GPIO_PIN_SET);
        bldc_flags_ptr->bldc_stop = 1;
    #endif
}
/**
 * @brief   换相
 * @param   BLDC_PARAMETERS_TypeDef *：bldc参数指针
 */
void bldc_turn(BLDC_PARAMETERS_TypeDef *bldc_param_ptr, BLDC_FLAGS_TypeDef *bldc_flags_ptr)
{
    if(bldc_param_ptr->phase_cnt > 6)
    {
        bldc_param_ptr->phase_cnt = 1;
    }else if(bldc_param_ptr->phase_cnt < 1)
    {
        bldc_param_ptr->phase_cnt = 6;
    }else{}
    switch(bldc_param_ptr->phase_cnt)
    {
        #if(DIRECTION == COUNTER_CLOCKWISE)
        case 1:
            {
                uh_vl();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_W;                   // 配置ADC的CHSELR寄存器，读取W相端电压
            }break;
        case 2:
            {
                uh_wl();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_V;                   // 配置ADC的CHSELR寄存器，读取V相端电压
            }break;
        case 3:
            {
                vh_wl();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_U;                   // 配置ADC的CHSELR寄存器，读取U相端电压

            }break;
        case 4:
            {
                vh_ul();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_W;                   // 配置ADC的CHSELR寄存器，读取W相端电压
            }break;
        case 5:
            {
                wh_ul();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_V;                   // 配置ADC的CHSELR寄存器，读取V相端电压
            }break;
        case 6:
            {
                wh_vl();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_U;                   // 配置ADC的CHSELR寄存器，读取U相端电压
            }break;
        default: bldc_stop(bldc_param_ptr ,bldc_flags_ptr); break;
        #elif(DIRECTION == CLOCKWISE)
        case 6:
            {
                uh_vl();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_W;                   // 配置ADC的CHSELR寄存器，读取W相端电压
            }break;
        case 5:
            {
                uh_wl();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_V;                   // 配置ADC的CHSELR寄存器，读取V相端电压
            }break;
        case 4:
            {
                vh_wl();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_U;                   // 配置ADC的CHSELR寄存器，读取U相端电压

            }break;
        case 3:
            {
                vh_ul();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_W;                   // 配置ADC的CHSELR寄存器，读取W相端电压
            }break;
        case 2:
            {
                wh_ul();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_V;                   // 配置ADC的CHSELR寄存器，读取V相端电压
            }break;
        case 1:
            {
                wh_vl();
                hadc1.Instance->CHSELR = ADC_CHSELR_CONFIG_U;                   // 配置ADC的CHSELR寄存器，读取U相端电压
            }break;
        default: bldc_stop(bldc_param_ptr ,bldc_flags_ptr); break;
        #endif
    }
}

