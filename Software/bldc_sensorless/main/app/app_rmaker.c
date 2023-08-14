#include "app_rmaker.h"
#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_types.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_standard_devices.h"
#include "esp_rmaker_schedule.h"
#include "esp_rmaker_scenes.h"
#include "esp_rmaker_console.h"
#include "esp_rmaker_ota.h"
#include "esp_rmaker_common_events.h"
#include "app_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_variable.h"
#include "string.h"

static const char *TAG = "APP_RMKER";
esp_rmaker_device_t *fan_device;
esp_rmaker_param_t *mode_param;  // 模式选择控件
esp_rmaker_param_t *speed_param; // 速度滑动条控件
esp_rmaker_param_t *power_param; // 启停控件
/**
 * @description: rmaker获取数据
 * @return {*}
 */
static esp_err_t app_rmaker_write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param, esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_POWER_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                 val.val.b ? "true" : "false", esp_rmaker_device_get_name(device),
                 esp_rmaker_param_get_name(param));
        // 设置启动和停止
        motorParameter.isStart = val.val.b;
        esp_rmaker_param_update_and_report(param, val);
    } else if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_SPEED_NAME) == 0) {
        // 获取转速
        ESP_LOGI(TAG, "Received Speed:%d", val.val.i);
        rmakerParameter.speed = val.val.i;
    } else if (strcmp(esp_rmaker_param_get_name(param), "Natural") == 0) {
        // 获取是否为自然风
        ESP_LOGI(TAG, "Received Mode:%d", val.val.b);
        if (motorParameter.isStart == STOP) {
            val = esp_rmaker_bool(false);
        } else {
            rmakerParameter.mode = val.val.b ? NATURAL : NORMAL;
        }
    }

    esp_rmaker_param_update_and_report(param, val);
    return ESP_OK;
}

/**
 * @description: rmaker回调事件
 * @param {void} *arg
 * @param {esp_event_base_t} event_base
 * @param {int32_t} event_id
 * @param {void} *event_data
 * @return {*}
 */
static void app_rmaker_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == RMAKER_EVENT) {
        switch (event_id) {
        case RMAKER_EVENT_INIT_DONE:
            ESP_LOGI(TAG, "RainMaker Initialised.");
            break;
        case RMAKER_EVENT_CLAIM_STARTED:
            ESP_LOGI(TAG, "RainMaker Claim Started.");
            break;
        case RMAKER_EVENT_CLAIM_SUCCESSFUL:
            ESP_LOGI(TAG, "RainMaker Claim Successful.");
            break;
        case RMAKER_EVENT_CLAIM_FAILED:
            ESP_LOGI(TAG, "RainMaker Claim Failed.");
            break;
        default:
            ESP_LOGW(TAG, "Unhandled RainMaker Event: %" PRIi32, event_id);
        }
    } else if (event_base == RMAKER_COMMON_EVENT) {
        switch (event_id) {
        case RMAKER_EVENT_REBOOT:
            ESP_LOGI(TAG, "Rebooting in %d seconds.", *((uint8_t *)event_data));
            break;
        case RMAKER_EVENT_WIFI_RESET:
            ESP_LOGI(TAG, "Wi-Fi credentials reset.");
            break;
        case RMAKER_EVENT_FACTORY_RESET:
            ESP_LOGI(TAG, "Node reset to factory defaults.");
            break;
        case RMAKER_MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected.");
            break;
        case RMAKER_MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected.");
            break;
        case RMAKER_MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT Published. Msg id: %d.", *((int *)event_data));
            break;
        default:
            ESP_LOGW(TAG, "Unhandled RainMaker Common Event: %" PRIi32, event_id);
        }
    } else if (event_base == APP_WIFI_EVENT) {
        switch (event_id) {
        case APP_WIFI_EVENT_QR_DISPLAY:
            ESP_LOGI(TAG, "Provisioning QR : %s", (char *)event_data);
            break;
        case APP_WIFI_EVENT_PROV_TIMEOUT:
            ESP_LOGI(TAG, "Provisioning Timed Out. Please reboot.");
            break;
        case APP_WIFI_EVENT_PROV_RESTART:
            ESP_LOGI(TAG, "Provisioning has restarted due to failures.");
            break;
        default:
            ESP_LOGW(TAG, "Unhandled App Wi-Fi Event: %" PRIi32, event_id);
            break;
        }
    } else if (event_base == RMAKER_OTA_EVENT) {
        switch (event_id) {
        case RMAKER_OTA_EVENT_STARTING:
            ESP_LOGI(TAG, "Starting OTA.");
            break;
        case RMAKER_OTA_EVENT_IN_PROGRESS:
            ESP_LOGI(TAG, "OTA is in progress.");
            break;
        case RMAKER_OTA_EVENT_SUCCESSFUL:
            ESP_LOGI(TAG, "OTA successful.");
            break;
        case RMAKER_OTA_EVENT_FAILED:
            ESP_LOGI(TAG, "OTA Failed.");
            break;
        case RMAKER_OTA_EVENT_REJECTED:
            ESP_LOGI(TAG, "OTA Rejected.");
            break;
        case RMAKER_OTA_EVENT_DELAYED:
            ESP_LOGI(TAG, "OTA Delayed.");
            break;
        case RMAKER_OTA_EVENT_REQ_FOR_REBOOT:
            ESP_LOGI(TAG, "Firmware image downloaded. Please reboot your device to apply the upgrade.");
            break;
        default:
            ESP_LOGW(TAG, "Unhandled OTA Event: %" PRIi32, event_id);
            break;
        }
    } else {
        ESP_LOGW(TAG, "Invalid event received!");
    }
}

/**
 * @description: rainmaker初始化
 * @return {*}
 */
void app_rmaker_init()
{
    /* Initialize NVS. */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /* Initialize rmaker. */
    esp_rmaker_console_init();
    app_wifi_init(); // BLE配网方式

    /* 注册Rmaker回调. */
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_EVENT, ESP_EVENT_ANY_ID, &app_rmaker_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_COMMON_EVENT, ESP_EVENT_ANY_ID, &app_rmaker_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(APP_WIFI_EVENT, ESP_EVENT_ANY_ID, &app_rmaker_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_OTA_EVENT, ESP_EVENT_ANY_ID, &app_rmaker_event_handler, NULL));

    /* 创建Rmaker节点. */
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "ESP RainMaker Device", "Switch");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        abort();
    }

    /* 创建设备*/
    fan_device = esp_rmaker_device_create("FanV3", ESP_RMAKER_DEVICE_FAN, NULL);
    esp_rmaker_device_add_cb(fan_device, app_rmaker_write_cb, NULL);
    // 添加控件-按钮 启动
    power_param = esp_rmaker_power_param_create(ESP_RMAKER_DEF_POWER_NAME, false);
    esp_rmaker_device_add_param(fan_device, power_param);
    // 添加控件-滑动条
    speed_param = esp_rmaker_param_create(ESP_RMAKER_DEF_SPEED_NAME, ESP_RMAKER_PARAM_SPEED,
                                          esp_rmaker_int(300), PROP_FLAG_READ | PROP_FLAG_WRITE);
    if (speed_param) {
        esp_rmaker_param_add_ui_type(speed_param, ESP_RMAKER_UI_SLIDER);
        esp_rmaker_param_add_bounds(speed_param, esp_rmaker_int(300), esp_rmaker_int(800), esp_rmaker_int(1));
    }
    esp_rmaker_device_add_param(fan_device, speed_param);

    //添加控件-文本
    esp_rmaker_param_t *text_param = esp_rmaker_param_create("Fan mode setting", ESP_RMAKER_UI_TEXT,
                                     esp_rmaker_str("Set natural or normal mode"), PROP_FLAG_READ);
    esp_rmaker_device_add_param(fan_device, text_param);

    // 添加控件-下拉 模式选择
    mode_param = esp_rmaker_power_param_create("Natural", false);
    esp_rmaker_device_add_param(fan_device, mode_param);

    esp_rmaker_device_assign_primary_param(fan_device, power_param); // 首页控件 电源
    esp_rmaker_node_add_device(node, fan_device);

    /* 启动rainmaker*/
    esp_rmaker_start();
    err = app_wifi_start(POP_TYPE_RANDOM);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        abort();
    }
}