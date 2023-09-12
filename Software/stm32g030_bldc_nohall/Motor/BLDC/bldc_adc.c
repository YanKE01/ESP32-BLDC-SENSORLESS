#include "bldc.h"
uint16_t adc1_value[ADC1_CHANNEL_CNT] = {0};

/**
 * @brief   平均滤波
 */
uint16_t filter_average(uint16_t *buffer_ptr, uint16_t n)
{
    uint16_t max_tmp = 0, min_tmp = 0, tmp = 0;
    max_tmp = min_tmp = tmp = *buffer_ptr;
    /* 获取传入数组中的最大值、最小值，并计算所有元素之和 */
    for(uint16_t i=1; i<n; i++)
    {
        if(max_tmp < *(buffer_ptr+i))
        {
            max_tmp = *(buffer_ptr+i);
        }
        if(min_tmp > *(buffer_ptr+i))
        {
            min_tmp = *(buffer_ptr+i);
        }else{}
        tmp += *(buffer_ptr+i);
    }
    tmp -= (max_tmp + min_tmp);
    if(n > 2)
    {
        tmp /= (n-2);
    }
    return tmp;
}


/**
 * @brief   数据缓存
 */
void buffer_storage(BUFFER_TypeDef *buffer_ptr, uint16_t value)
{
    buffer_ptr->array[buffer_ptr->idx] = value;     // 把数据存入缓存区
    if(++buffer_ptr->idx >= 16)
    {
        buffer_ptr->idx = 0;
    }
}


void adc_value_transfer(BLDC_ADC_SAMPLE_TypeDef *bldc_adc_sample_ptr,
                        BLDC_PARAMETERS_TypeDef *bldc_param_ptr)
{
    bldc_adc_sample_ptr->bus_volt = adc1_value[3]*VBUS_ADC_COFF;
    
    if(bldc_param_ptr->status == BLDC_RUN_3)
    {
        /* 电机运行状态下的电流要减去偏置电流 */
        if(adc1_value[1] > bldc_adc_sample_ptr->bus_offset_current)
        {
            bldc_adc_sample_ptr->bus_current = (adc1_value[1] - bldc_adc_sample_ptr->bus_offset_current) * CURRENT_BUS_ADC_COFF;    
        }
    }else if(bldc_param_ptr->status == BLDC_STOP_5)
    {
        bldc_adc_sample_ptr->bus_offset_current = adc1_value[1];    // 电机非运行状态下的电流值不进行转换，直接使用rawdata
    }
}
