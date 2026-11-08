#ifndef SENSOR_H
#define SENSOR_H

#include "esp_err.h"

typedef struct {
    float lux;
    float percentage;
    float resistance;
} light_data_t;

esp_err_t sensor_init(void);

esp_err_t sensor_read(light_data_t *data);

#endif