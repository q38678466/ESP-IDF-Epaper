#include "deep_sleep.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "app_config.h"
#define TAG "deep_sleep"
static TimerHandle_t my_timer = NULL;  // 定时器句柄


static deep_sleep_cb user_deep_sleep_callback = NULL;

/// @brief 进入深度睡眠的回调函数回调
/// @param xTimer xx
static void timer_callback(TimerHandle_t xTimer)
{
    ESP_LOGW(TAG, "Timer triggered! Entering deep sleep...");
    if(user_deep_sleep_callback != NULL) {
        user_deep_sleep_callback();  // 调用用户定义的回调函数
    }
    // 设置唤醒源：EXT0模式（只能单个引脚）
    esp_sleep_enable_ext0_wakeup(WAKEUP_PIN, 0);  // 0 = 低电平唤醒
    esp_sleep_enable_timer_wakeup(RTC_WAKEUP_TIME);  // 1分钟
    esp_deep_sleep_start();
}

/// @brief 打印唤醒原因
void printf_weakeup_reason()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_GPIO:
            ESP_LOGI(TAG, "Wakeup caused by GPIO");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            ESP_LOGI(TAG, "Wakeup caused by timer");
            break;
        default:
            ESP_LOGI(TAG, "Wakeup was not caused by deep sleep: %d", wakeup_reason);
            break;
    }
}

/// @brief 创建进入深度睡眠的倒计时时钟
/// @param time_in_ms 倒计时时间，单位：毫秒
/// @return 
esp_err_t create_deep_sleep_timer(uint64_t time_in_ms)
{
    my_timer = xTimerCreate("SleepTimer",pdMS_TO_TICKS(time_in_ms),pdTRUE,NULL,timer_callback);
    if (!my_timer)
    {
        ESP_LOGE(TAG, "Failed to create sleep timer");
        return ESP_FAIL;
    }

    if (xTimerStart(my_timer, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start timer!");
        return ESP_FAIL;
    }
    return ESP_OK;
    
}
/// @brief 进入深度睡眠的计时清0
/// @return 
esp_err_t reset_deep_sleep_timer_count()
{
    if (xTimerReset(my_timer, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to reset timer!");
        return ESP_FAIL;
    }
    return ESP_OK;
}

/// @brief 修改深度睡眠定时器的倒计时
/// @param time_in_ms 新的倒计时时间，单位：毫秒
esp_err_t change_deep_sleep_timer_period(uint64_t time_in_ms)
{
    if (my_timer == NULL) {
        ESP_LOGE(TAG, "Timer not created!");
        return ESP_FAIL;
    }

    if (xTimerChangePeriod(my_timer, pdMS_TO_TICKS(time_in_ms), 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to change timer period!");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Deep sleep timer period changed to %llu ms", time_in_ms);

    reset_deep_sleep_timer_count();
    return ESP_OK;
}

void register_deep_sleep_callback(deep_sleep_cb cb)
{
    user_deep_sleep_callback = cb;
}