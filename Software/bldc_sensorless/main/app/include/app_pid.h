#ifndef _APP_PID_H
#define _APP_PID_H

#include "stdint.h"
#include "esp_attr.h"

/*定义PID参数相关宏*/
#define KP 0.00300f /* P参数*/
#define KI 0.00005f /* I参数*/

/*定义位置PID参数相关宏*/
/*PID结构体*/
typedef struct {
    int SetPoint;    /* 设定目标 */
    float ActualValue; /* 实际值 */
    float Ui;          /* 比例项 */
    float Up;          /* 积分项 */
    float Ud;          /* 微分项 */
    float Proportion;  /* 比例常数 P */
    float Integral;    /* 积分常数 I */
    float Error;       /* Error[-1] */
    float IngMin;
    float IngMax;
    float OutMin;
    float OutMax;
} PID_TypeDef;

extern PID_TypeDef speedPid;

/**
 * @description: pid初始化
 * @return {*}
 */
void app_pid_init();

/**
 * @description: pid运算
 * @param {PID_TypeDef} *PID
 * @param {float} currentVale
 * @return {*}
 */
int IRAM_ATTR app_pid_operation(PID_TypeDef *PID, int currentVale);

#endif
