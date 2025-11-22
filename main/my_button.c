#include "my_button.h"
#include "button_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "iot_button.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "app_config.h"
#include "my_sht30.h"
#include "deep_sleep.h"
#include "my_wifi.h"
#define TAG "my_button"


extern float temperature, humidity;
static void main_button_event_cb(void *arg, void *data)
{
    iot_button_print_event((button_handle_t)arg);

    int repeat = iot_button_get_repeat((button_handle_t)arg);
    if(repeat >= 3)
    {
        ESP_LOGI(TAG, "enter wifi config mode");
        enter_wifi_config_mode_reset();
    }
    
    my_sht30_get_data(&temperature, &humidity);
    printf("temperature:%.2f, humidity:%.2f\n", temperature, humidity);
    reset_deep_sleep_timer_count();

}

static void up_button_event_cb(void *arg, void *data)
{
    iot_button_print_event((button_handle_t)arg);
}

static void down_button_event_cb(void *arg, void *data)
{
    iot_button_print_event((button_handle_t)arg);
}

static void mid_button_event_cb(void *arg, void *data)
{
    iot_button_print_event((button_handle_t)arg);
}

void my_button_init(void)
{
    button_config_t btn_cfg = {0};
    button_gpio_config_t gpio_cfg = {
        .gpio_num = MAIN_BUTTON_PIN,
        .active_level = BUTTON_ACTIVE_LEVEL,
        .enable_power_save = true,
    };

    button_handle_t main_btn;
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &main_btn);
    gpio_cfg.gpio_num = UP_BUTTON_PIN;
    button_handle_t up_btn;
    ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &up_btn);
    gpio_cfg.gpio_num = DOWN_BUTTON_PIN;
    button_handle_t down_btn;
    ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &down_btn);
    gpio_cfg.gpio_num = MID_BUTTON_PIN;
    button_handle_t mid_btn;
    ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &mid_btn);

    ret = iot_button_register_cb(main_btn, BUTTON_SINGLE_CLICK, NULL, main_button_event_cb, NULL);
    ret = iot_button_register_cb(main_btn, BUTTON_PRESS_REPEAT_DONE, NULL, main_button_event_cb, NULL);
    ret = iot_button_register_cb(up_btn, BUTTON_SINGLE_CLICK, NULL, up_button_event_cb, NULL);
    ret = iot_button_register_cb(down_btn, BUTTON_SINGLE_CLICK, NULL, down_button_event_cb, NULL);
    ret = iot_button_register_cb(mid_btn, BUTTON_SINGLE_CLICK, NULL, mid_button_event_cb, NULL);
    
}