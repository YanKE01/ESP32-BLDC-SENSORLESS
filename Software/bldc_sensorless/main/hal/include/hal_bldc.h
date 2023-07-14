#ifndef _APP_BLDC_H
#define _APP_BLDC_H

#include <stdint.h>

#define HIN_1 GPIO_NUM_17
#define HIN_2 GPIO_NUM_16
#define HIN_3 GPIO_NUM_15
#define LIN_1 GPIO_NUM_12
#define LIN_2 GPIO_NUM_11
#define LIN_3 GPIO_NUM_10

#define _LEDC_TIMER LEDC_TIMER_0
#define _LEDC_MODE LEDC_LOW_SPEED_MODE
#define _LEDC_DUTY_RES LEDC_TIMER_11_BIT // Set duty resolution to 11 bits
#define _LEDC_DUTY (2048 - 1)            // Duty Max = 2^11 = 2048
#define _LEDC_FREQUENCY (20 * 1000)      // Frequency in Hertz. Set frequency at 20 kHz

void hal_bldc_hal_init();
void hal_bldc_hal_test(uint8_t status, float duty);

#endif
