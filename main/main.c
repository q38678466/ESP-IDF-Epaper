/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <inttypes.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include <sys/time.h>


#include "e_paper.h"
#include "Pic.h"
#include "app_config.h"
#include "my_sht30.h"
#include "deep_sleep.h"
#include "my_button.h"

#define TAG "main"

//http://acs.m.taobao.com/gw/mtop.common.getTimestamp/

// 全局互斥锁句柄
SemaphoreHandle_t epd_mutex;
float temperature, humidity;
/// @brief 设置系统时区为中国标准时间
/// @param  
void set_system_time_country(void)
{
    setenv("TZ", "CST-8", 1);
    tzset();
}

/// @brief 设置系统时间
/// @param timestamp 时间戳
void set_system_time(time_t timestamp) {
    struct timeval tv;
    tv.tv_sec = timestamp;   // 秒
    tv.tv_usec = 0;          // 微秒

    if (settimeofday(&tv, NULL) == 0) {
        ESP_LOGI(TAG,"system time set successfully");
    } else {
        ESP_LOGE(TAG,"system time set failed");
    }
}


void epaper_main_task(void *pvParameter)
{
    
    EPD_Init();
    EPD_FastMode1Init();
    EPD_Display_Clear();
    EPD_FastUpdate();//更新画面显示
    EPD_Clear_R26H();
    EPD_ShowPicture(0,88,32,32,gImage_temp,BLACK);
    EPD_ShowPicture(0,120,32,32,gImage_himi,BLACK);
    EPD_ShowSensor_Data(32,90,temperature,4,2,24,BLACK);
    EPD_ShowSensor_Data(32,122,humidity,4,2,24,BLACK);
    EPD_ShowWatch(12,10,0,4,2,48,BLACK);
    EPD_ShowNum_Two(135,38,0,12,BLACK);
    EPD_Display(ImageBW);
    EPD_PartUpdate();
    // vTaskDelay(2000/portTICK_PERIOD_MS);
    while(1)
    {
      /*********************局刷模式**********************/
        
        struct timeval tv_now;
        gettimeofday(&tv_now, NULL);
        struct tm tm_now;
        localtime_r(&tv_now.tv_sec, &tm_now);
        
        float time_val = tm_now.tm_hour + tm_now.tm_min / 100.0;
        u16 time_sec = tv_now.tv_sec % 60;

        my_sht30_get_data(&temperature, &humidity);
        // 获取显示锁进行局部更新
        if (xSemaphoreTake(epd_mutex, portMAX_DELAY) == pdTRUE) {
            EPD_ShowWatch(12,10,time_val,4,2,48,BLACK);
            EPD_ShowNum_Two(135,38,time_sec,12,BLACK);
            EPD_ShowSensor_Data(32,90,temperature,4,2,24,BLACK);
            EPD_ShowSensor_Data(32,122,humidity,4,2,24,BLACK);
            EPD_Display(ImageBW);
            EPD_PartUpdate();
            xSemaphoreGive(epd_mutex); // 释放锁
        }
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}

/// @brief 执行完回调函数才会进入睡眠
/// @param  
void enter_deep_sleep_cb(void)
{
    // struct tm tm_now;
    // localtime_r(1643731200, &tm_now);

    if (xSemaphoreTake(epd_mutex, portMAX_DELAY) == pdTRUE) {
        Paint_Clear(WHITE);
        EPD_Display(ImageBW);
        EPD_PartUpdate();
        EPD_ShowPicture(0,0,152,152,gImage_cat,BLACK);
        EPD_Display(ImageBW);
        EPD_PartUpdate();
        EPD_DeepSleep();
    }
    // vTaskDelay(5000/portTICK_PERIOD_MS);
}




void app_main(void)
{
    
    printf_weakeup_reason();
    set_system_time_country();
    set_system_time(1761128634);
    my_button_init();
    my_sht30_init();
    epaper_spi_init();
    // 创建互斥锁
    epd_mutex = xSemaphoreCreateMutex();
    if (epd_mutex == NULL) {
        ESP_LOGE(TAG, "互斥锁创建失败!\n");
        return;
    }
    xTaskCreate(epaper_main_task, "epaper_main_task", 2048, NULL, 10, NULL);
    
    ESP_ERROR_CHECK(create_deep_sleep_timer(ENTER_DEEP_SLEEP_TIME));
    register_deep_sleep_callback(enter_deep_sleep_cb);
    
    
}
