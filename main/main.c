
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "wifi.h"
#include "websocket.h"
#include "sensor.h"
#include "lcd.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(wifi_init());
    ESP_ERROR_CHECK(sensor_init());
    ESP_ERROR_CHECK(lcd_init());
    ESP_ERROR_CHECK(websocket_start());

    light_data_t leitura;

    // Controle independente do intervalo do LCD
    int64_t ultimo_update_lcd = 0;
    const int64_t intervalo_lcd = 1000000; // 1 segundo

    while (1)
    {
        esp_err_t ret = sensor_read(&leitura);

        if (ret == ESP_OK)
        {
            char mensagem[32];

            // Formata mensagem para o WebSocket
            snprintf(mensagem, sizeof(mensagem), "%.2f", leitura.lux);

            // Verifica se chegou o momento de atualizar o LCD
            int64_t agora = esp_timer_get_time();

            if (agora - ultimo_update_lcd >= intervalo_lcd)
            {
                char linha1[17];
                char linha2[17];

                snprintf(linha1, sizeof(linha1), "%.2f lux", leitura.lux);
                snprintf(linha2, sizeof(linha2), "%.2f %%", leitura.percentage);

                lcd_clear();

                lcd_print(linha1);
                lcd_set_cursor(0, 1);
                lcd_print(linha2);

                ultimo_update_lcd = agora;
            }

            ESP_LOGI(TAG, "Luminosidade: %.2f lux", leitura.lux);

            // Envio pelo WebSocket
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

        // Intervalo principal: 100 ms
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
