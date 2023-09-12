#include "bldc.h"
void bldc_ctrl(BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
               BLDC_FLAGS_TypeDef *bldc_flags_ptr,
               BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_param_ptr,
               BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
               PID_PARAMETERS_TypeDef *pid_param_ptr)
{
    switch(bldc_param_ptr->status)
    {
        case BLDC_INIT_0:       // 初始化
        {
            bldc_init(bldc_param_ptr, bldc_startup_param_ptr, bldc_zc_param_ptr, pid_param_ptr, bldc_flags_ptr);
        }break;
        case BLDC_ALIGNMENT_1:
        {
            
        }break;
        case BLDC_DRAG_2:
        {

        }break;
        case BLDC_RUN_3:
        {
            bldc_speed_ctrl(bldc_param_ptr, bldc_zc_param_ptr, pid_param_ptr);
        }break;
        case BLDC_RESET_4:
        {

        }break;
        case BLDC_STOP_5:
        {
            bldc_stop(bldc_param_ptr, bldc_flags_ptr);
        }break;
        default: bldc_stop(bldc_param_ptr, bldc_flags_ptr); break;
    }
}
