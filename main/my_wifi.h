#ifndef MY_WIFI_H
#define MY_WIFI_H

#include "esp_system.h"

void wifi_init(void);
void enter_wifi_config_mode_reset();
bool wifi_is_got_ip();

#endif // MY_WIFI_H