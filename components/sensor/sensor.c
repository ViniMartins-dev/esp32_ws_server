#include "sensor.h"

#include <math.h>

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#define SENSOR_ADC_UNIT       ADC_UNIT_1
#define SENSOR_ADC_CHANNEL    ADC_CHANNEL_6
#define SENSOR_ADC_ATTEN      ADC_ATTEN_DB_12
#define SENSOR_ADC_WIDTH      ADC_BITWIDTH_12

#define ADC_MAX               4095.0f
#define VCC                   3.3f
#define R_FIXED               10000.0f
#define MAX_LUX               200.0f

static const char *TAG = "SENSOR";

static adc_oneshot_unit_handle_t adc_handle;

esp_err_t sensor_init(void)
{
    // Configuração do ADC
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = SENSOR_ADC_UNIT,
    };

    esp_err_t ret = adc_oneshot_new_unit(
        &init_config,
        &adc_handle
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao inicializar ADC");
        return ret;
    }

    // Configuração do canal
    adc_oneshot_chan_cfg_t channel_config = {
        .atten = SENSOR_ADC_ATTEN,
        .bitwidth = SENSOR_ADC_WIDTH,
    };

    ret = adc_oneshot_config_channel(
        adc_handle,
        SENSOR_ADC_CHANNEL,
        &channel_config
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao configurar canal ADC");
        return ret;
    }

    ESP_LOGI(TAG, "Sensor inicializado");

    return ESP_OK;
}

esp_err_t sensor_read(light_data_t *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    int raw_input;

    // Leitura do ADC
    esp_err_t ret = adc_oneshot_read(
        adc_handle,
        SENSOR_ADC_CHANNEL,
        &raw_input
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erro na leitura ADC");
        return ret;
    }

    // Conversão ADC para tensão (aproximada)
    float vadc = (raw_input * VCC) / ADC_MAX;

    // Evita divisão por zero
    if (vadc <= 0.0f || vadc >= VCC) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    // Resistência do LDR
    // Circuito: 3.3V -> 10k -> GPIO34 -> LDR -> GND
    data->resistance = R_FIXED * vadc / (VCC - vadc);

    // Estimativa de luminosidade
    data->lux = 500.0f * powf(
        1000.0f / data->resistance,
        1.4f
    );

    // Conversão para porcentagem
    data->percentage = (data->lux / MAX_LUX) * 100.0f;

    if (data->percentage > 100.0f) {
        data->percentage = 100.0f;
    }

    if (data->percentage < 0.0f) {
        data->percentage = 0.0f;
    }

    return ESP_OK;
}