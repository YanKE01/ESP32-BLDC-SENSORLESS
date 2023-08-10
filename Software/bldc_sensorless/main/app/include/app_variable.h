#ifndef _APP_VARIABLE_H
#define _APP_VARIABLE_H

#include <stdint.h>

enum Dir {
    CW = 0, // 顺时针
    CCW,    // 逆时针
};

enum StartorStop {
    STOP = 0, // 停止
    START,    // 启动
};

enum ControlMode {
    NORMAL = 0, // 直接速度控制
    NATURAL,    // 自然风控制
};

typedef struct {
    uint32_t hallState; // 霍尔传感器状态
    uint32_t pwmDuty;   // 占空比
    uint8_t dir;        // 方向
    uint8_t isStart;    // 是否启动
    uint16_t maxDuty;   // 最大占空比
    uint8_t lock;       // 堵转标志位
    int changeIndex;    // 调整索引
    float currentVbus;  // 电源电压
} MotorParameter;

typedef struct {
    uint8_t power; // 电源控件
    int speed;     // 风速控件
    uint8_t mode;  // 模式控件：自然风/直接控制
} RmakerParameter;

extern MotorParameter motorParameter;
extern RmakerParameter rmakerParameter;
/**
 * @description: 变量初始化.
 * @return {*}
 */
void app_variable_init(void);

#endif
