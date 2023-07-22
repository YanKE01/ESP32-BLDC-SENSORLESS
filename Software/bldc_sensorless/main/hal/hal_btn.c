#include "hal_btn.h"
#include "esp_log.h"
#include "iot_button.h"
#include "driver/gpio.h"
#include "app_variable.h"

static button_handle_t btn_handle;
static const char *TAG = "HAL_BTN";

/**
 * @description: 按键单击回调
 * @param {void} *arg
 * @param {void} *data
 * @return {*}
 */
static void button_single_click_cb(void *arg, void *data)
{
    motorParameter.isStart = !motorParameter.isStart;

    ESP_LOGI(TAG, "BTN___STATE:%d", motorParameter.isStart);
}

/**
 * @description: 按键初始化
 * @return {*}
 */
void hal_btn_init(void)
{
    button_config_t cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 2000,
        .short_press_time = 500,
        .gpio_button_config = {
            .gpio_num = GPIO_NUM_0,
            .active_level = 0,
        },
    };

    btn_handle = iot_button_create(&cfg);
    iot_button_register_cb(btn_handle, BUTTON_SINGLE_CLICK, button_single_click_cb, NULL);
}