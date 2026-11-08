#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"

#include "wifi.h"
#include "websocket.h"
#include "sensor.h"

#include "esp_log.h"
#include "esp_err.h"

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(wifi_init());

    ESP_ERROR_CHECK(websocket_start());

    while (1)
    {
        int leitura = sensor_read();

        char mensagem[32];

        snprintf(mensagem, sizeof(mensagem), "%d", leitura);

        ESP_LOGI("MAIN", "Leitura do sensor: %d", leitura);

        esp_err_t ret = websocket_send(mensagem);

        if (ret != ESP_OK)
        {
            ESP_LOGW("MAIN", "Falha ao enviar: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}