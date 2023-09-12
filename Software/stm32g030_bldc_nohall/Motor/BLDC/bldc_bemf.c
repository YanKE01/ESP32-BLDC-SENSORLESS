#include "bldc.h"

/**
 * @brief   反电动势过零点检测
 */
void bemf_check(BLDC_FLAGS_TypeDef *bldc_flags_ptr,
                BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr,
                BLDC_ZC_PARAMETERS_TypeDef* bldc_zc_parameters_ptr)
{
    static uint16_t vbus_half = 0;          // 存放 1/2 的总线电压
    if(bldc_flags_ptr->angle_mask == 0)     // 等待续流时间结束后获取端电压
    {
        bldc_parameters_ptr->bemf = adc1_value[0];
        vbus_half = (uint16_t)(adc1_value[3] / 2);
    }
    if(bldc_flags_ptr->change_phase==0 && bldc_flags_ptr->angle_mask==0)
    {
        switch(bldc_parameters_ptr->phase_cnt)
        {
            case 1:
            {
                if(bldc_parameters_ptr->bemf < vbus_half)
                {
                    if(++(bldc_zc_parameters_ptr->blanking_cnt) >= Delay_Filter)
                    {
                        bldc_zc_parameters_ptr->blanking_cnt = 0;
                        bldc_parameters_ptr->phase_cnt = 2;
                        calc_speed_time(bldc_zc_parameters_ptr, bldc_flags_ptr);
                    }
                }
            }break;
            case 2:
            {
                if(bldc_parameters_ptr->bemf > vbus_half)
                {
                    if(++(bldc_zc_parameters_ptr->blanking_cnt) >= Delay_Filter)
                    {
                        bldc_zc_parameters_ptr->blanking_cnt = 0;
                        bldc_parameters_ptr->phase_cnt = 3;
                        calc_speed_time(bldc_zc_parameters_ptr, bldc_flags_ptr);
                    }
                }
            }break;
            case 3:
            {
                if(bldc_parameters_ptr->bemf < vbus_half)
                {
                    if(++(bldc_zc_parameters_ptr->blanking_cnt) >= Delay_Filter)
                    {
                        bldc_zc_parameters_ptr->blanking_cnt = 0;
                        bldc_parameters_ptr->phase_cnt = 4;
                        calc_speed_time(bldc_zc_parameters_ptr, bldc_flags_ptr);
                    }
                }
            }break;
            case 4:
            {
                if(bldc_parameters_ptr->bemf > vbus_half)
                {
                    if(++(bldc_zc_parameters_ptr->blanking_cnt) >= Delay_Filter)
                    {
                        bldc_zc_parameters_ptr->blanking_cnt = 0;
                        bldc_parameters_ptr->phase_cnt = 5;
                        calc_speed_time(bldc_zc_parameters_ptr, bldc_flags_ptr);
                    }
                }
            }break;
            case 5:
            {
                if(bldc_parameters_ptr->bemf < vbus_half)
                {
                    if(++(bldc_zc_parameters_ptr->blanking_cnt) >= Delay_Filter)
                    {
                        bldc_zc_parameters_ptr->blanking_cnt = 0;
                        bldc_parameters_ptr->phase_cnt = 6;
                        calc_speed_time(bldc_zc_parameters_ptr, bldc_flags_ptr);
                    }
                }
            }break;
            case 6:
            {
                if(bldc_parameters_ptr->bemf > vbus_half)
                {
                    if(++(bldc_zc_parameters_ptr->blanking_cnt) >= Delay_Filter)
                    {
                        bldc_zc_parameters_ptr->blanking_cnt = 0;
                        bldc_parameters_ptr->phase_cnt = 1;
                        calc_speed_time(bldc_zc_parameters_ptr, bldc_flags_ptr);
                    }
                }
            }break;
            default: break;
        }
    }
}
