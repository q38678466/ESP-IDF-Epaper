#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H
#include <sys/time.h>
#include "esp_system.h"
void set_system_time_country(void);
void set_system_time(time_t timestamp);
void  get_network_time();

#endif // SYSTEM_TIME_H