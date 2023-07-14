#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal_bldc.h"

void app_main(void)
{
    hal_bldc_hal_init();
    hal_bldc_hal_test(1, 0.5);

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}