#include "app_variable.h"
#include "hal_bldc.h"

MotorParameter motorParameter;

void app_variable_init(void)
{
    motorParameter.dir = CW; // 默认顺时针 CW顺时针 CCW逆时针
    motorParameter.hallState = 0;
    motorParameter.pwmDuty = 0;
    motorParameter.isStart = 0;
    motorParameter.maxDuty = _LEDC_DUTY;
    motorParameter.isStart = STOP;
}