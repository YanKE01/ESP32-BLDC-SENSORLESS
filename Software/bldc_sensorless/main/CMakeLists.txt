set(HAL_FILES "hal/hal_bldc.c")
set(APP_FILES "app/app_variable.c" "app/app_pid.c")

idf_component_register(
  SRCS
  "main.c"
  ${HAL_FILES}
  ${APP_FILES}
  INCLUDE_DIRS
  "."
  "hal/include"
  "app/include")
