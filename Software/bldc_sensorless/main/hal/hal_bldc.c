#include "hal_bldc.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "app_variable.h"
#include "app_pid.h"

ledc_channel_t lec_channel[3] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2};
SimpleOpen simpleOpen = {0, 0, 0, 0, 0};
HallLessParameter hallLessParameter = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 0, 0, 0, 0, 0, 0, 0};
int speedCount = 0;     // 用于统计高电平次数,计算速度
int zeroStableFlag = 0; // 过零点稳定标志位

/**
 * @description: 硬件初始化
 * @return {*}
 */
void hal_bldc_hal_init()
{
    // 下管控制IO初始化
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LIN_1);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = (1ULL << LIN_2);
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = (1ULL << LIN_3);
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_7); // heart led
    gpio_config(&io_conf);

    // 反电势IO初始化 INPUT_PULLUP
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    io_conf.pin_bit_mask = (1ULL << HALL_1);
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = (1ULL << HALL_2);
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = (1ULL << HALL_3);
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_0);
    gpio_config(&io_conf);

    // ledc初始化
    ledc_timer_config_t ledc_timer = {
        .speed_mode = _LEDC_MODE,
        .duty_resolution = _LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = _LEDC_FREQUENCY, // Set output frequency at 20 kHz
        .clk_cfg = LEDC_USE_APB_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    int pins[3] = {HIN_1, HIN_2, HIN_3}; // save pins

    ledc_channel_config_t ledc_channel_cfg;
    for (int i = 0; i < 3; i++)
    {
        ledc_channel_cfg.speed_mode = _LEDC_MODE;
        ledc_channel_cfg.timer_sel = LEDC_TIMER_0;
        ledc_channel_cfg.duty = 0;
        ledc_channel_cfg.hpoint = 0;
        ledc_channel_cfg.intr_type = LEDC_INTR_DISABLE;
        ledc_channel_cfg.channel = lec_channel[i];
        ledc_channel_cfg.gpio_num = pins[i];
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_cfg));
    }

    hal_bldc_stop();
}

/**
 * @description: 设置led
 * @param {uint8_t} status
 * @return {*}
 */
void hal_flag_led(uint8_t status)
{
    gpio_set_level(GPIO_NUM_7, status);
}
/**
 * @description: bldc硬件测试
 * @param {uint8_t} status
 * @param {float} duty: 0~1
 * @return {*}
 */
void hal_bldc_hal_test(uint8_t status, float duty)
{
    // 控制IO
    gpio_set_level(LIN_1, status);
    gpio_set_level(LIN_2, status);
    gpio_set_level(LIN_3, status);
    gpio_set_level(GPIO_NUM_7, status);

    // 控制PWM
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[0], (uint32_t)((_LEDC_DUTY * duty))));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[0]));
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[1], (uint32_t)((_LEDC_DUTY * duty))));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[1]));
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[2], (uint32_t)((_LEDC_DUTY * duty))));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[2]));
}

/**
 * @description: bldc关闭 强制下管关闭
 * @return {*}
 */
void hal_bldc_stop()
{
    // ledc停止输出
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[0], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[0])); // U
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[1], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[1])); // V
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[2], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[2])); // W

    // 关闭下桥
    gpio_set_level(LIN_1, 1);
    gpio_set_level(LIN_2, 1);
    gpio_set_level(LIN_3, 1);
}

/**
 * @description: u项上管导通和v项下管导通
 * @return {*}
 */
void hal_UphaseH_VphaseL(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[0], motorParameter.pwmDuty));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[0])); // U
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[1], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[1])); // V
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[2], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[2])); // W

    // v下管导通
    gpio_set_level(LIN_1, 1); // U
    gpio_set_level(LIN_2, 0); // V
    gpio_set_level(LIN_3, 1); // W
}

/**
 * @description: u项上管导通和w项下管导通
 * @return {*}
 */
void hal_UphaseH_WphaseL(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[0], motorParameter.pwmDuty));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[0])); // U
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[1], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[1])); // V
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[2], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[2])); // W

    // w下管导通
    gpio_set_level(LIN_1, 1); // U
    gpio_set_level(LIN_2, 1); // V
    gpio_set_level(LIN_3, 0); // W
}

/**
 * @description: v项上管导通和w项下管导通
 * @return {*}
 */
void hal_VphaseH_WphaseL(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[0], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[0])); // U
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[1], motorParameter.pwmDuty));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[1])); // V
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[2], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[2])); // W

    // w下管导通
    gpio_set_level(LIN_1, 1); // U
    gpio_set_level(LIN_2, 1); // V
    gpio_set_level(LIN_3, 0); // W
}

/**
 * @description: v项上管导通和u项下管导通
 * @return {*}
 */
void hal_VphaseH_UphaseL(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[0], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[0])); // U
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[1], motorParameter.pwmDuty));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[1])); // V
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[2], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[2])); // W

    // u下管导通
    gpio_set_level(LIN_1, 0); // U
    gpio_set_level(LIN_2, 1); // V
    gpio_set_level(LIN_3, 1); // W
}

/**
 * @description: w项上管导通和u项下管导通
 * @return {*}
 */
void hal_WphaseH_UphaseL(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[0], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[0])); // U
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[1], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[1])); // V
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[2], motorParameter.pwmDuty));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[2])); // W

    // u下管导通
    gpio_set_level(LIN_1, 0); // U
    gpio_set_level(LIN_2, 1); // V
    gpio_set_level(LIN_3, 1); // W
}

/**
 * @description: w项上管导通和u项下管导通
 * @return {*}
 */
void hal_WphaseH_VpahseL(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[0], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[0])); // U
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[1], 0));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[1])); // V
    ESP_ERROR_CHECK(ledc_set_duty(_LEDC_MODE, lec_channel[2], motorParameter.pwmDuty));
    ESP_ERROR_CHECK(ledc_update_duty(_LEDC_MODE, lec_channel[2])); // W

    // v下管导通
    gpio_set_level(LIN_1, 1); // U
    gpio_set_level(LIN_2, 0); // V
    gpio_set_level(LIN_3, 1); // W
}

/**
 * @brief 逆时针换向 HALLLESS 513264,5对应就是array的第5个，1对应array的第1个
 */
void (*ccwArray[6])(void) = {
    &hal_UphaseH_WphaseL,
    &hal_VphaseH_UphaseL,
    &hal_VphaseH_WphaseL,
    &hal_WphaseH_VpahseL,
    &hal_UphaseH_VphaseL,
    &hal_WphaseH_UphaseL,
};

/**
 * @brief 顺时针换向 HALLLESS 546231,5对应就是array的第5个，4对应array的第4个
 *
 */
void (*cwArray[6])(void) = {
    &hal_UphaseH_VphaseL,
    &hal_VphaseH_WphaseL,
    &hal_UphaseH_WphaseL,
    &hal_WphaseH_UphaseL,
    &hal_WphaseH_VpahseL,
    &hal_VphaseH_UphaseL,
};

/**
 * @description: 修改上管占空比
 * @return {*}
 */
void hal_bldc_change_voltage()
{
    switch (simpleOpen.voltageChangeCount)
    {
    case 1:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 4;
        break;
    case 2:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 4;
        break;
    case 3:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 4;
        break;
    case 4:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 4;
        break;
    case 5:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 4;
        break;
    case 6:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 4;
        break;
    case 7:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 3;
        break;
    case 8:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 3;
        break;
    case 9:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 3;
        break;
    case 10:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 3;
        break;
    case 11:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 3;
        break;
    case 12:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 2;
        break;
    case 13:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 2;
        break;
    case 14:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 2;
        break;
    case 15:
        motorParameter.pwmDuty = motorParameter.maxDuty / 2 / 2;
        break;
    default:
        break;
    }
}

/**
 * @description: 六步换向逻辑
 * @return {*}
 */
void hal_bldc_six_step_operation()
{
    if (motorParameter.dir == CCW)
    {
        switch (simpleOpen.nextPhaseCount)
        {
        case 0:
            hal_UphaseH_VphaseL();
            break;
        case 1:
            hal_UphaseH_WphaseL();
            break;
        case 2:
            hal_VphaseH_WphaseL();
            break;
        case 3:
            hal_VphaseH_UphaseL();
            break;
        case 4:
            hal_WphaseH_UphaseL();
            break;
        case 5:
            hal_WphaseH_VpahseL();
            break;

        default:
            break;
        }
    }
    else
    {
        switch (simpleOpen.nextPhaseCount)
        {
        case 0:
            hal_UphaseH_VphaseL();
            break;
        case 1:
            hal_WphaseH_VpahseL();
            break;
        case 2:
            hal_WphaseH_UphaseL();
            break;
        case 3:
            hal_VphaseH_UphaseL();
            break;
        case 4:
            hal_VphaseH_WphaseL();
            break;
        case 5:
            hal_UphaseH_WphaseL();
            break;

        default:
            break;
        }
    }
}

/**
 * @description: 强行固定一项测试
 * @return {*}
 */
void hal_bldc_test(void)
{
    motorParameter.pwmDuty = 200;
    for (int i = 0; i < 1000; i++)
    {
        hal_WphaseH_VpahseL();
    }
    hal_bldc_stop();
    motorParameter.pwmDuty = 0;
}

/**
 * @description: emf软件滤波器
 * @param {int} *valueLpf
 * @param {int} value
 * @return {*}
 */
void hal_bldc_lpf(int *valueLpf, int value)
{
    static int valuePrev = 0;

    *valueLpf = (int)(0.9 * valuePrev + 0.1 * value);

    valuePrev = *valueLpf;
}

/**
 * @description: emf边沿监测
 * @param {uint8_t} val
 * @return {*}
 */
uint8_t hal_bldc_umef_edge(uint8_t val)
{
    /* 主要是检测val信号从0 - 1 在从 1 - 0的过程，即高电平所持续的过程 */
    static uint8_t oldval = 0;
    if (oldval != val)
    {
        oldval = val;

        if (val == 0)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    return 2;
}

/**
 * @description: 判断是否进入无感状态
 * @return {*}
 */
uint8_t hal_bldc_sensor_less_operation(void)
{
    static uint8_t edgeFlag = 0; /* 过零信号稳定标志 */

    speedCount++; // 只统计1->0这段时间的高电平,其余时间会被清空

    // 存放过零状态,每次与上真实的IO状态,若8次都是1 表示检测信号为1,记为稳定
    hallLessParameter.queueState[0] = hallLessParameter.queueState[0] << 1;
    hallLessParameter.queueState[1] = hallLessParameter.queueState[1] << 1;
    hallLessParameter.queueState[2] = hallLessParameter.queueState[2] << 1;

    hallLessParameter.queueState[0] |= gpio_get_level(HALL_1);
    hallLessParameter.queueState[1] |= gpio_get_level(HALL_2);
    hallLessParameter.queueState[2] |= gpio_get_level(HALL_3);

    // 判断三相是否稳定
    for (int i = 0; i < 3; i++)
    {
        if ((hallLessParameter.queueState[i] & 0XFFFF) == 0XFFFF)
        {
            hallLessParameter.queueFilterState[i] = 1;
        }
        else if ((hallLessParameter.queueState[i] & 0xFFFF) == 0x0000)
        {
            hallLessParameter.queueFilterState[i] = 0;
        }
        else
        {
            hallLessParameter.filterFailedCount++; // 不稳定计数,后续会用作转速为0的判断
            return 0;
        }
    }

    /*速度测量*/
    hallLessParameter.filterEdge = hal_bldc_umef_edge(hallLessParameter.queueFilterState[0]);

    if (hallLessParameter.filterEdge == 0)
    {
        // 说明从1->0,开始统计过零时间
        if (zeroStableFlag >= 4)
        {
            if (motorParameter.dir == CCW)
            {
                // ft/(2*n*c) * 60; 其中ft为计数频率,就是周期中断的频率，n为极对数,c是记的次数
                hallLessParameter.speedRpm = (uint32_t)(((10 * 1000) / (2 * 5 * speedCount)) * 60); // 4极对
            }
            else
            {
                hallLessParameter.speedRpm = -(uint32_t)(((10 * 1000) / (2 * 5 * speedCount)) * 60); // 4极对
            }

            hal_bldc_lpf(&hallLessParameter.speedRpm, hallLessParameter.speedRpm);
        }
        hallLessParameter.filterDelay = speedCount / 10; // 高电平时间记为180度,滞后30度，除以6即可，可以让延迟时间更短,避免硬件问题
        hallLessParameter.filterFailedCount = 0;
        speedCount = 0;
        zeroStableFlag++;
    }

    if (hallLessParameter.filterEdge == 1)
    {
        speedCount = 0;
        hallLessParameter.filterFailedCount = 0;
    }

    if (hallLessParameter.filterEdge == 2)
    {
        // 反电动势没有变化,没有检测到过零信号
        hallLessParameter.filterFailedCount++;

        if (hallLessParameter.filterFailedCount > 15000)
        {
            // 反电势一直没有变化，说明已经堵转或者异常
            hallLessParameter.filterFailedCount = 0;
            hallLessParameter.speedRpm = 0;
            motorParameter.pwmDuty = 0;
            motorParameter.isStart = STOP; // 电机停止
            hal_bldc_stop();
            motorParameter.lock = 1; // 堵转标志成立
            hal_flag_led(0);
        }
    }

    // 过零控制
    if (zeroStableFlag >= 4)
    {
        static int filterCount = 0;
        // 稳定的过零信号
        zeroStableFlag = 4;
        edgeFlag++; // 稳定后,旋转两圈进入控制

        if (edgeFlag >= 2)
        {
            edgeFlag = 2; // 只有第一次进入过零控制才有,进入完成直接进入
            hallLessParameter.hallLessValue = (hallLessParameter.queueFilterState[0]) | (hallLessParameter.queueFilterState[1] << 1) | (hallLessParameter.queueFilterState[2] << 2);

            if (hallLessParameter.hallLessValue <= 0 || hallLessParameter.hallLessValue > 6)
            {
                return 0;
            }

            if (hallLessParameter.hallLessValue != hallLessParameter.hallLessValuePrev)
            {
                filterCount++; // 延迟30度的时间
            }

            if (filterCount >= hallLessParameter.filterDelay)
            {
                filterCount = 0;
                if (hallLessParameter.hallLessValue != hallLessParameter.hallLessValuePrev)
                {
                    // 发生变化，即可换向
                    if (motorParameter.dir == CW)
                    {
                        cwArray[hallLessParameter.hallLessValue - 1]();
                    }
                    else
                    {
                        ccwArray[hallLessParameter.hallLessValue - 1]();
                    }
                }
                hallLessParameter.hallLessValuePrev = hallLessParameter.hallLessValue;
            }
        }

        return 1;
    }

    return 0;
}

/**
 * @description: 无感BLDC主循环
 * @return {*}
 */
void hal_bldc_main_loop(void)
{
    if (motorParameter.isStart == START)
    {
        switch (simpleOpen.runStep)
        {
        case 0:
            motorParameter.pwmDuty = motorParameter.maxDuty / 8;
            hal_UphaseH_VphaseL(); // 固定到这一项
            simpleOpen.delayCount = 0;
            simpleOpen.nextPhaseTime = 900; // 假设值(需要调试),相与相之间切换的时间，没有进入无感状态时
            simpleOpen.runStep = 1;
            simpleOpen.voltageChangeCount = 0; // 用于开环调节电压
            zeroStableFlag = 0;                // 清除过零标志位
            break;

        case 1:
            // 此环节为延迟计数，以达到换向时间的要求
            simpleOpen.delayCount++;
            if (simpleOpen.delayCount >= simpleOpen.nextPhaseTime)
            {
                // 达到换向时间
                simpleOpen.delayCount = 0;
                simpleOpen.runStep = 2;
            }

            if (hal_bldc_sensor_less_operation() == 1)
            {
                simpleOpen.runStep = 3;
            }

            break;
        case 2:
            // 若达到换向时间，则按照顺序换相即可，同时缩短换相时间
            simpleOpen.nextPhaseTime -= simpleOpen.nextPhaseTime / 15 + 1;
            simpleOpen.voltageChangeCount++;
            hal_bldc_change_voltage(); // 尝试增大占空比

            if (simpleOpen.nextPhaseTime < 180)
            {
                simpleOpen.nextPhaseTime = 180;
            }

            simpleOpen.runStep = 1;

            simpleOpen.nextPhaseCount++; // 标记即将进入下一项 六步换相的顺序

            if (simpleOpen.nextPhaseCount == 6)
            {
                simpleOpen.nextPhaseCount = 0; // 新的一轮六步换相
            }
            hal_bldc_six_step_operation(); // 开环强托的六部换向
            break;
        case 3:
            hal_flag_led(1);
            hal_bldc_sensor_less_operation();
            break;
        default:
            break;
        }
    }
    else
    {
        hal_bldc_stop();
        // 清空无感标志位
        simpleOpen.runStep = 0;
        simpleOpen.delayCount = 0;
        simpleOpen.nextPhaseCount = 0;
        simpleOpen.voltageChangeCount = 0;
        hallLessParameter.stableFlag = 0;
        hallLessParameter.filterFailedCount = 0;
        speedPid.Ui = 0;
        speedPid.SetPoint = 400.0f;
        motorParameter.changeIndex = 0;
        hal_flag_led(0);
    }
}
