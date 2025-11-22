#ifndef DEEP_SLEEP_H
#define DEEP_SLEEP_H
#include "esp_system.h"
typedef void (*deep_sleep_cb)(void);   // 定义一个回调函数类型


esp_err_t create_deep_sleep_timer(uint64_t time_in_ms);
void printf_weakeup_reason();
esp_err_t reset_deep_sleep_timer_count();
void register_deep_sleep_callback(deep_sleep_cb cb);
esp_err_t change_deep_sleep_timer_period(uint64_t time_in_ms);

#endif /* DEEP_SLEEP_H */