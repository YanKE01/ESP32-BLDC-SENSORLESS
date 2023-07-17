#include "app_pid.h"
#include "app_variable.h"

PID_TypeDef speedPid;

#define LIMIT_OUT(var, max, min)               \
    {                                          \
        (var) = (var) > (max) ? (max) : (var); \
        (var) = (var) < (min) ? (min) : (var); \
    }

void app_pid_init()
{
    speedPid.SetPoint = 800.0f;  // 目标值
    speedPid.ActualValue = 0.0f; // pid运算值
    speedPid.Ui = 0.0f;
    speedPid.Up = 0.0f;
    speedPid.Error = 0.0f;                              // 误差
    speedPid.Proportion = KP;                           // 比例项输出
    speedPid.Integral = KI;                             // 积分项输出
    speedPid.IngMax = 1.0f * motorParameter.maxDuty;    // 限制积分输出
    speedPid.IngMin = -1.0f * motorParameter.maxDuty;   // 限制积分输出
    speedPid.OutMax = 0.9f * (motorParameter.maxDuty);  // 限制PID输出
    speedPid.OutMin = -0.9f * (motorParameter.maxDuty); // 限制PID输出
}

int app_pid_operation(PID_TypeDef *PID, float currentVale)
{
    if (motorParameter.dir == CW && currentVale < 0)
    {
        currentVale = -currentVale;
    }

    PID->Error = (float)(PID->SetPoint - currentVale); /* 偏差值 */
    PID->Up = PID->Proportion * PID->Error;
    PID->Ui += (PID->Error * PID->Integral);
    LIMIT_OUT(PID->Ui, PID->IngMax, PID->IngMin);
    PID->ActualValue = PID->Up + PID->Ui + PID->Ud;
    LIMIT_OUT(PID->ActualValue, PID->OutMax, PID->OutMin);
    return ((int)(PID->ActualValue)); /* 返回实际控制数值 */
}