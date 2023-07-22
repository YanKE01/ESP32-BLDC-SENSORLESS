set(HAL_FILES "hal/hal_bldc.c" "hal/hal_btn.c")
set(APP_FILES "app/app_variable.c" "app/app_pid.c" "app/app_wind.c")
set(INC_FILES "hal/include" "app/include")

idf_component_register(
  SRCS
  "main.c"
  ${HAL_FILES}
  ${APP_FILES}
  INCLUDE_DIRS
  "."
  ${INC_FILES})
