#include "app_variable.h"
#include "hal_bldc.h"
#include "time.h"
#include "stdlib.h"

MotorParameter motorParameter;
RmakerParameter rmakerParameter;

/**
 * @description: app变量初始化
 * @return {*}
 */
void app_variable_init(void)
{
    /*电机参数初始化.*/
    motorParameter.dir = CW; // 默认顺时针 CW顺时针 CCW逆时针.注：相对转向，与实际UVW接线有关
    motorParameter.hallState = 0;
    motorParameter.pwmDuty = 0;
    motorParameter.maxDuty = _LEDC_DUTY;
    motorParameter.isStart = STOP;
    motorParameter.lock = 0;
    motorParameter.changeIndex = 0;
    motorParameter.currentVbus = 0;

    /*Rmaker参数初始化.*/
    rmakerParameter.mode = NORMAL;
    rmakerParameter.power = STOP;
    rmakerParameter.speed = 300;
    srand(time(0)); // 设置随机数种子,风速波动
}