#ifndef __HWBOARD_PARAMETERS_DEFINE_H__
#define __HWBOARD_PARAMETERS_DEFINE_H__
#include <stm32g0xx_hal.h>

/* UVW三相的下MOS控制GPIO */
#define U_L_GPIO_PORT       GPIOA
#define U_L_GPIO_PIN        GPIO_PIN_5
#define V_L_GPIO_PORT       GPIOA
#define V_L_GPIO_PIN        GPIO_PIN_4
#define W_L_GPIO_PORT       GPIOA
#define W_L_GPIO_PIN        GPIO_PIN_12
/* 总线电压，总线电流的ADC采样 相关参数 */
#define ADC_RESOLUTION              4095U               // ADC分辨率
#define ADC_REF_VOLT                3.3F                // ADC参考电压
#define VBUS_R_LOW                  0.51F               // 电压总线采样，下电阻510ohm
#define VBUS_R_HIGH                 10.0F               // 电压总线采样，上电阻10Kohm
#define VBUS_ADC_COFF               (float)((VBUS_R_HIGH+VBUS_R_LOW)*ADC_REF_VOLT/VBUS_R_LOW/ADC_RESOLUTION)
#define CURRENT_BUS_R               0.005F              // 总线电流采样电阻
#define CURRENT_BUS_OPA_GAIN        52.0F               // 运放倍数
#define CURRENT_BUS_ADC_COFF        (float)(ADC_REF_VOLT/ADC_RESOLUTION/CURRENT_BUS_OPA_GAIN/CURRENT_BUS_R)

#endif
