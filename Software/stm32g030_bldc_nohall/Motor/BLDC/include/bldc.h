#ifndef __BLDC_H__
#define __BLDC_H__
#include <stdint.h>
#include <stm32g0xx_hal.h>
#include "bldc_parameters_define.h"
#include "hwboard_parameters_define.h"
#include "motor_parameters_define.h"
#include "adc.h"
#include "tim.h"
#include "gpio.h"

#define ADC1_CHANNEL_CNT                4           // ADC1启用的通道数 分别采集 （UVW端电压 总线电流 MCU温度 总线电压）

#define BLDCSTOP                        1           // BLDC停止
#define BLDCSTART                       2           // BLDC启动
#define BLDCRUN                         3           // BLDC运行
#define BLDCSTART_ERR                   4           // 电机启动错误
#define BLDCTIME_ERR                    5
#define BLDCRUNTIME_ERR                 6           // 电机运行错误

/* PWM占空比 */
#define PWM_DUTY_100                    2000U       
#define PWM_DUTY_95                     1900U
#define PWM_DUTY_90                     1800U
#define PWM_DUTY_85                     1700U
#define PWM_DUTY_80                     1600U
#define PWM_DUTY_75                     1500U
#define PWM_DUTY_70                     1400U
#define PWM_DUTY_65                     1300U
#define PWM_DUTY_60                     1200U
#define PWM_DUTY_55                     1100U
#define PWM_DUTY_50                     1000U
#define PWM_DUTY_45                     900U
#define PWM_DUTY_40                     800U
#define PWM_DUTY_35                     700U
#define PWM_DUTY_30                     600U
#define PWM_DUTY_25                     500U
#define PWM_DUTY_20                     400U
#define PWM_DUTY_15                     300U
#define PWM_DUTY_10                     200U
#define PWM_DUTY_5                      100U
#define PWM_DUTY_MAX                    PWM_DUTY_95
#define PWM_DUTY_MIN                    PWM_DUTY_15
/* 转速范围 */
#define SPEED_MAX                       4000U
#define SPEED_MIN                       500U

#define BLDC_DELAY_TIME                 2000U
/* ADC CHSELR寄存器值 用来选择ADC的通道 */
#define ADC_CHSELR_CONFIG_U             0X00009009U
#define ADC_CHSELR_CONFIG_V             0X0000900AU
#define ADC_CHSELR_CONFIG_W             0X0000900CU


/* 电感法 */
#define LOCK_DUTY                       PWM_DUTY_90             // 脉冲定位时，注入脉冲的占空比
#define LONG_PULSE_CNT                  10                      // 电容充电时间计数
#define SHORT_PULSE_CNT                 4                       // 脉冲时间计数
/* 启动 */
#define ALIGNMENTNMS                    (0)                     // 定位时间 
#define ALIGNMENTDUTY                   (200)                   // 定位力矩   
#define RAMP_TIM_STA                    (190)                   // 爬坡开始步进时间  原本200  数值越小启动较快  ，容易过流
#define RAMP_TIM_END                    (20)                    // 爬坡结束步进时间 30 根据负载调  如果带载  这个值适当大点
#define RAMP_TIM_STEP                   (9)                     // 爬坡步进时间递减增量 --跟随RAMP_TIM_STA调整
#define RAMP_DUTY_STA                   (PWM_DUTY_5)            // 爬坡开始力矩      
#define RAMP_DUTY_END                   (PWM_DUTY_20)           // 爬坡结束力矩       
#define RAMP_DUTY_INC                   (13)                    // 爬坡步进力矩增量--数值太小启动不起来 太大容易过流    
#define START_TIMES                     (2)                     // 启动次数(暂时不用)
#define Run_Times                       (100)                   // 强拖到闭环稳定时间  n*62.5us  (暂时不用)
#define Maul_AutoTime                   (30)                    // 进来先给初值  强拖切自动运行  参数重要(暂时不用)
/* 停机方式 */
#define FREEDOWN                        (0)                     // 自由停车
#define BRAKEDOWN                       (1)                     // 强刹
#define STOPMODE                        (FREEDOWN)              // 刹车方式选择
/* 控制模式选择 */
#define OPEN_LOOP_Halless               (1)                     // 1:开环运行
#define CLOSED_SPEEDLOOP_Halless        (2)                     // 2：速度闭环
#define Control_Mode                    (CLOSED_SPEEDLOOP_Halless)     // 模式选择
/* 加速限制 */
#define ADD_DUTY1                       (1)                     // 开环加速幅度
#define ADD_DUTY2                       (2)                     // 开环加速幅度
#define ADD_DUTY3                       (3)                     // 开环加速幅度
 
#define LOW_DUTY_COUNT                  (2)                     // 低占空比开环加速加速度
#define HIGH_DUTY_COUNT                 (2)                     // 高占空比开环加速加速度
#define DUTYTHRESHOLD1                  (0.5*PWM_DUTY_100)      // 阈值设定1
#define DUTYTHRESHOLD2                  (0.7*PWM_DUTY_100)      // 阈值设定2
#define DUTYTHRESHOLD3                  (1*PWM_DUTY_100)        // 阈值设定3
/* 续流屏蔽和换相时间补偿 */
#define Low_DutyMask_Time               (2)                     // 低占空比续流屏蔽时间   运行过程中无故硬件过流  调整该值
#define High_DutyMask_Time              (3)                     // 高占空比续流屏蔽时间 --运行过程中无故硬件过流  调整该值
#define Delay_Filter                    (2)                     // 反电动势采集滤波深度  --影响波形的超前和滞后
/* 调速方式 */
#define DIRECT_GIVE                     (0)                     // 直接给定
#define STEPLESS_SPEED                  (1)                     // 无极变速
#define ADJ_MODE                        (DIRECT_GIVE) 
/* 保护参数设置区 */
#define MAX_BUS_VOLT                    (15.0F)                 // 过压
#define MIN_BUS_VOLT                    (10.0F)                 // 低压
#define MAX_BUS_CURRENT                 10.0F                   // 过流
/* 转速计算因子 */
#define SPEED_FACTOR                    1000000/POLE_PAIRS*60/6 // 60 * SYS_CLK / (6 * POLE_PAIR * TIM14_PSC)
/* 一阶滤波函数 */
#define FIRST_ORDER_LFP_CACL(Xn, Yn_1, a)   Yn_1 = (1-a)*Yn_1 + a*Xn
/* 速度PID */
#define INIT_PURPOSE                    0                       
#define RUN_PURPOSE                     1
#define SPEEDLOOPCNT                    3                       // 速度占空比调整周期
/* 正反转控制 */
#define COUNTER_CLOCKWISE               0                       // 逆时针转向
#define CLOCKWISE                       1                       // 顺时针转向
#define DIRECTION                       (CLOCKWISE)             // 转子转向
typedef enum
{
    BLDC_INIT_0 = 0, 
    BLDC_ALIGNMENT_1 = 1,
    BLDC_DRAG_2 = 2, 
    BLDC_RUN_3 = 3, 
    BLDC_RESET_4 = 4, 
    BLDC_STOP_5 = 5                     // 电机停止工作
}BLDC_STATUS_TypeDef;

typedef enum
{
    RUNNORMAL_0 = 0,                    // 电机无故障
    OVER_UNDER_VOLT_1 = 1,              // 总线电压过压或欠压
    OVER_CURRENT_2 = 2,                 // 总线电流过流
    OVER_TEMPERA_3 = 3,                 // MCU过温
    BLDC_STALL_4 = 4,                   // 电机堵转
    HW_OVER_CURRENT_5 = 5,              // 硬件过流
    OVERTIME_PHASE_CHANGE_6 = 6,        // 超时换相错误
}BLDC_FAULTS_TypeDef;

/* BLDC标志位结构体 */
typedef struct 
{
    unsigned sys_start: 1;
    unsigned bldc_stop: 1;
    unsigned angle_mask: 1;
    unsigned change_phase: 1;
    unsigned flag_speed_time: 1;
}BLDC_FLAGS_TypeDef;

/* BLDC参数结构体 */
typedef struct
{
    uint8_t control_mode;               // BLDC控制方式
    BLDC_STATUS_TypeDef status;         // BLDC当前状态
    BLDC_FAULTS_TypeDef faults;         // BLDC故障状态
    uint16_t user_require_speed;        // 用户需求速度
    uint16_t theory_speed;              // BLDC理论转速
    uint16_t actual_speed;              // BLDC实际转速
    uint16_t adc_avg_speed;             // BLDC平均转速
    uint16_t last_speed;
    uint16_t open_speed;            
    uint16_t duty;                      // BLDC PWM占空比
    uint16_t order;
    float current;
    uint16_t step_cnt;
    uint8_t phase_cnt;
    uint8_t last_phase;
    uint8_t next_phase;
    uint16_t bemf;
}BLDC_PARAMETERS_TypeDef;

/* BLDC过零控制参数结构体 */
typedef struct
{
    uint16_t drag_time;             // 强拖时间
    uint16_t phase_err_cnt;         // 换相错误计数
    uint16_t adc_time_cnt;
    uint16_t timer_30ut_cnt;
    uint16_t blanking_cnt;
    uint16_t stop_time;
    uint16_t speed_time_cnt;
    uint16_t speed_time;
    uint16_t speed_time_tmp;
    uint16_t speed_time_sum;
    uint16_t delay_time30;          // 换相延时时间
    uint16_t mask_time;             // 换相屏蔽时间
}BLDC_ZC_PARAMETERS_TypeDef;

/* 启动相关参数结构体 */
typedef struct 
{
    uint8_t flag_adc;               // 
    uint8_t flag_charge;            // 电容充电标志位
    uint16_t adc_check_buf[6];      // 存放六个脉冲激励下的总线电流值
    uint8_t pos_idx;                // 第 pos_idx 个脉冲，0~5
    uint8_t pos_check_stage;        // 脉冲注入阶段
    uint8_t phase_cnt;
    uint8_t initial_stage;          // 转子初始位置信息
    uint8_t charge_cnt;             // 充电时间计数
}BLDC_STARTUP_PARAMETERS_TypedDef;


/* BLDC保护标志位结构体，1表示开启保护，0表示关闭保护 */
typedef struct
{
    unsigned flag_hw_over_current: 1;   // 硬件过流标志位，1表示开启硬件过流保护，0表示关闭硬件过流保护
}BLDC_PROTECTOR_TypeDef;
/*  */
typedef struct
{
    uint16_t array[16];
    uint8_t idx;
}BUFFER_TypeDef;

/* ADC采样参数结构体 */
typedef struct
{
    float bus_volt;             // 母线电压
    float bus_current;          // 母线电流
    float u_phase_vlot;         // U相电压
    float v_phase_vlot;         // V相电压
    float w_phase_vlot;         // W相电压
    float bus_offset_current;   // 母线偏移电流
    float mcu_tempera;          // MCU温度
    BUFFER_TypeDef buffer;      // 存放总线电流
    float coeff_filter_k1;      // 一阶低通滤波器系数 1
    float coeff_filter_k2;      // 一阶低通滤波器系数 2
}BLDC_ADC_SAMPLE_TypeDef;

/* PID参数结构体 */
typedef struct
{
    float kp;                   // 比例项系数
    float ki;                   // 积分项系数
    float kd;                   // 微分项系数
    uint16_t max_value;         // 积分上限
    uint16_t min_value;         // 积分下限
    float fpall_err;            // 累计误差
    float error;                // 误差
    float out;                  // 输出
    float purpose;              // INIT_PURPOSE RUN_PURPOSE 标志位
    float reference;            
    float ui;                   // 积分项
    uint16_t max;               // 输出上限
    uint16_t min;               // 输出下限
}PID_PARAMETERS_TypeDef;


extern uint16_t adc1_value[ADC1_CHANNEL_CNT];
extern BLDC_PARAMETERS_TypeDef bldc_parameters;
extern BLDC_ZC_PARAMETERS_TypeDef bldc_zc_parameters;
extern BLDC_FLAGS_TypeDef bldc_flags;
extern BLDC_STARTUP_PARAMETERS_TypedDef bldc_startup_parameters;
extern BLDC_PROTECTOR_TypeDef bldc_protector;
extern BLDC_ADC_SAMPLE_TypeDef bldc_adc_sample;
extern PID_PARAMETERS_TypeDef pid_speed;

/* BLDC相关参数初始化函数 */
void bldc_parameters_init(BLDC_PARAMETERS_TypeDef *bldc_param_ptr);
void bldc_flags_init(BLDC_FLAGS_TypeDef *bldc_flags_ptr);
void bldc_zc_parameters_init(BLDC_ZC_PARAMETERS_TypeDef * bldc_zc_param_ptr);
void bldc_startup_parameters_init(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_param_ptr);
void pid_speed_init(PID_PARAMETERS_TypeDef *pid_speed_ptr);
/* 中断回调函数 */
void DMA1_Channel1_XferCpltCallback(struct __DMA_HandleTypeDef *hdma);

void bldc_turn(BLDC_PARAMETERS_TypeDef *bldc_param_ptr, BLDC_FLAGS_TypeDef *bldc_flags_ptr);
void bldc_stop(BLDC_PARAMETERS_TypeDef *bldc_param_ptr, BLDC_FLAGS_TypeDef *bldc_flags_ptr);
void align_pos_check_process(BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_parameters_ptr,
                            BLDC_PROTECTOR_TypeDef *bldc_protector_ptr,
                            BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr,
                            BLDC_FLAGS_TypeDef *bldc_flags_ptr,
                            BLDC_ZC_PARAMETERS_TypeDef * bldc_zc_parameters_ptr);
void startup_drag(BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr,
                  BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_parameters_ptr,
                  BLDC_FLAGS_TypeDef *bldc_flags_ptr); 
void bemf_check(BLDC_FLAGS_TypeDef *bldc_flags_ptr,
                BLDC_PARAMETERS_TypeDef *bldc_parameters_ptr,
                BLDC_ZC_PARAMETERS_TypeDef* bldc_zc_parameters_ptr);
void calc_speed_time(BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                     BLDC_FLAGS_TypeDef *bldc_flags_ptr);
void all_mos_off(void);
void bldc_init(BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
               BLDC_STARTUP_PARAMETERS_TypedDef* bldc_startup_param_ptr,
               BLDC_ZC_PARAMETERS_TypeDef* bldc_zc_param_ptr,
               PID_PARAMETERS_TypeDef *pid_param_ptr,
               BLDC_FLAGS_TypeDef *bldc_flags_ptr);

void bldc_speed_ctrl(BLDC_PARAMETERS_TypeDef *bldc_param_ptr, BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr, PID_PARAMETERS_TypeDef *pid_param_ptr);
void bldc_ctrl(BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
               BLDC_FLAGS_TypeDef *bldc_flags_ptr,
               BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_param_ptr,
               BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
               PID_PARAMETERS_TypeDef *pid_param_ptr);
void adc_value_transfer(BLDC_ADC_SAMPLE_TypeDef *bldc_adc_sample_ptr,
                        BLDC_PARAMETERS_TypeDef *bldc_param_ptr);
void bldc_pid_cacl(int32_t ref_speed, int32_t actual_speed, PID_PARAMETERS_TypeDef *pid_param_ptr, BLDC_PARAMETERS_TypeDef *bldc_param_ptr);
void calc_avg_speed_time(BLDC_FLAGS_TypeDef *bldc_flags_ptr, BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr);
void fault_dection(BLDC_ADC_SAMPLE_TypeDef *bldc_adc_sample_ptr,
                   BLDC_ZC_PARAMETERS_TypeDef *bldc_zc_param_ptr,
                   BLDC_PARAMETERS_TypeDef *bldc_param_ptr,
                   BLDC_FLAGS_TypeDef *bldc_flags_ptr,
                   BLDC_STARTUP_PARAMETERS_TypedDef *bldc_startup_param_ptr,
                   PID_PARAMETERS_TypeDef *pid_param_ptr);
#endif
