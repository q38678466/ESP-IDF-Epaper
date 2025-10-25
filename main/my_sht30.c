#include "my_sht30.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"
#include "i2c_bus.h"
#include "esp_system.h"
#include "sht3x.h"
#include "esp_err.h"
#include "app_config.h"



static i2c_bus_handle_t i2c_bus = NULL;
static sht3x_handle_t sht3x = NULL;

void my_sht30_init(void)
{
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_bus = i2c_bus_create(I2C_MASTER_NUM, &conf);
    sht3x = sht3x_create(i2c_bus, SHT3x_ADDR_PIN_SELECT_VSS);
    sht3x_set_measure_mode(sht3x, SHT3x_SINGLE_MEDIUM_DISABLED);
}


void my_sht30_get_data(float *Tem_val, float *Hum_val)
{
    // sht3x_get_humiture(sht3x, Tem_val, Hum_val);
    sht3x_set_measure_mode(sht3x, SHT3x_SINGLE_MEDIUM_DISABLED);
    sht3x_get_single_shot(sht3x, Tem_val, Hum_val);
}
