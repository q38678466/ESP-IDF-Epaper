#ifndef MY_HTTP_SERVER_H
#define MY_HTTP_SERVER_H

#include "esp_http_server.h"
#include "esp_system.h"

httpd_handle_t start_webserver(bool dns_enable);

#endif // MY_HTTP_SERVER_H