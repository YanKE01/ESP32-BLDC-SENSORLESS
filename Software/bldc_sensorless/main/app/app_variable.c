#include "app_variable.h"
#include "hal_bldc.h"

MotorParameter motorParameter;

void VariableInit(void)
{
    motorParameter.dir = CCW; // 默认顺时针
    motorParameter.hallState = 0;
    motorParameter.pwmDuty = 0;
    motorParameter.isStart = 0;
    motorParameter.maxDuty = _LEDC_DUTY;
    motorParameter.isStart = STOP;
}