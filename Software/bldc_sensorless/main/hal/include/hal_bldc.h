#ifndef _APP_BLDC_H
#define _APP_BLDC_H

#include <stdint.h>

#define HIN_1 GPIO_NUM_17
#define HIN_2 GPIO_NUM_16
#define HIN_3 GPIO_NUM_15

#define LIN_1 GPIO_NUM_12
#define LIN_2 GPIO_NUM_11
#define LIN_3 GPIO_NUM_10

#define HALL_1 GPIO_NUM_3
#define HALL_2 GPIO_NUM_46
#define HALL_3 GPIO_NUM_9

#define _LEDC_TIMER LEDC_TIMER_0
#define _LEDC_MODE LEDC_LOW_SPEED_MODE
#define _LEDC_DUTY_RES LEDC_TIMER_11_BIT // Set duty resolution to 11 bits
#define _LEDC_DUTY (2048 - 1)            // Duty Max = 2^11 = 2048
#define _LEDC_FREQUENCY (20 * 1000)      // Frequency in Hertz. Set frequency at 20 kHz

typedef struct {
    /* data */
    int runStep;
    int delayCount;         // 延迟计数
    int nextPhaseTime;      // 切换到下一项的时间
    int nextPhaseCount;     // 换向计数
    int voltageChangeCount; // 改变电压计数
} SimpleOpen;

typedef struct {
    /* data */
    uint8_t hallLessState[3];     // 三相反电动式状态
    uint16_t queueState[3];       // 三相滤波
    uint16_t queueFilterState[3]; // 三相滤波后的状态
    uint16_t filterEdge;          // 滤波后边沿检测
    int speedRpm;                 // 电机转速
    uint16_t filterFailedCount;   // 不稳定计数
    uint16_t filterDelay;         // 延迟换向的时间
    uint8_t hallLessValue;        // 三相反电势状态组合值
    uint8_t hallLessValuePrev;    // 三相反电势状态组合值,前一时刻
    int stableFlag;               // 稳定状态
    int errorHallLessCount;       // 错误的无感计数
} HallLessParameter;

extern HallLessParameter hallLessParameter;
extern SimpleOpen simpleOpen;

void hal_bldc_hal_init();
void hal_bldc_hal_test(uint8_t status, float duty);
void hal_bldc_test(void);
void hal_bldc_stop();
void hal_flag_led(uint8_t status);
void hal_bldc_main_loop(void);

#endif
