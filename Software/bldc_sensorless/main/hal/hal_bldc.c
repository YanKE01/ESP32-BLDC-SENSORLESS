#include "hal_bldc.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "app_variable.h"

ledc_channel_t lec_channel[3] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2};

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
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << LIN_2);
    gpio_config(&io_conf);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << LIN_3);
    gpio_config(&io_conf);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_7); // heart led
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