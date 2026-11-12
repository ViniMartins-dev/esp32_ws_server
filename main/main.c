
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "wifi.h"
#include "websocket.h"
#include "sensor.h"

#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(wifi_init());
    ESP_ERROR_CHECK(sensor_init());
    ESP_ERROR_CHECK(websocket_start());

    light_data_t leitura;

    while (1)
    {
        esp_err_t ret = sensor_read(&leitura);

        if (ret == ESP_OK)
        {
            char mensagem[32];

            snprintf(mensagem, sizeof(mensagem), "%.2f", leitura.lux);

            ESP_LOGI(TAG, "Luminosidade: %.2f lux", leitura.lux);

            ret = websocket_send(mensagem);

            if (ret != ESP_OK)
            {
                ESP_LOGW(TAG, "Falha ao enviar: %s",
                         esp_err_to_name(ret));
            }
        }
        else
        {
            ESP_LOGW(TAG, "Falha na leitura: %s",
                     esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
