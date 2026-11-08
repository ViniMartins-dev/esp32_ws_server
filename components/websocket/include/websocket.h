
#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "esp_err.h"

esp_err_t websocket_start(void);

esp_err_t websocket_send(const char *message);

#endif
