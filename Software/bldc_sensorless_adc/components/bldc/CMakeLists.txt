idf_component_register(SRCS "bldc_driver.c" "bldc_startup.c" "bldc.c" "bldc_startup.c" "bldc_adc.c" "bldc_common.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_adc)
