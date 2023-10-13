#include "bldc_adc.h"

void bldc_adc_init(bldc_adc_t *bldc_adc)
{
    // adc init
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &bldc_adc->adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_0,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(bldc_adc->adc1_handle, ADC_CHANNEL_0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(bldc_adc->adc1_handle, ADC_CHANNEL_1, &config));
}

int bldc_adc_read_amp(bldc_adc_t *bldc_adc)
{
    int value = 0;
    adc_oneshot_read(bldc_adc->adc1_handle, ADC_CHANNEL_0, &value);
    return value;
}

int bldc_adc_read_vbus(bldc_adc_t *bldc_adc)
{
    int value = 0;
    adc_oneshot_read(bldc_adc->adc1_handle, ADC_CHANNEL_1, &value);
    return value;
}