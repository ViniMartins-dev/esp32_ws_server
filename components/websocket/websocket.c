
#include <stdlib.h>
#include <string.h>

#include "websocket.h"

#include "esp_log.h"
#include "esp_http_server.h"

static const char *TAG = "WEBSOCKET";

static httpd_handle_t server = NULL;
static int client_fd = -1;

/*
 * Recebe mensagens do cliente.
 */
static esp_err_t websocket_handler(httpd_req_t *req)
{
    // Registra o cliente que enviou a mensagem
    client_fd = httpd_req_to_sockfd(req);

    ESP_LOGI(TAG, "Cliente registrado! FD: %d", client_fd);

    httpd_ws_frame_t ws_pkt = {0};

    // Descobre o tamanho da mensagem
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Erro ao receber mensagem");
        return ret;
    }

    // Aloca memória para receber a mensagem
    uint8_t *buffer = calloc(1, ws_pkt.len + 1);

    if (buffer == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    ws_pkt.payload = buffer;

    // Recebe o conteúdo
    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);

    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Mensagem recebida: %s", (char *)buffer);

        // Devolve a mensagem ao cliente
        ret = httpd_ws_send_frame(req, &ws_pkt);
    }

    free(buffer);

    return ret;
}

/*
 * Envia uma mensagem de forma assíncrona.
 */
static void websocket_send_async(void *arg)
{
    char *message = (char *)arg;

    if (client_fd < 0)
    {
        free(message);
        return;
    }

    // Verifica se o cliente ainda está conectado
    if (httpd_ws_get_fd_info(server, client_fd) !=
        HTTPD_WS_CLIENT_WEBSOCKET)
    {
        ESP_LOGW(TAG, "Cliente desconectado");

        client_fd = -1;

        free(message);
        return;
    }

    httpd_ws_frame_t ws_pkt = {
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t *)message,
        .len = strlen(message)};

    esp_err_t ret = httpd_ws_send_frame_async(
        server,
        client_fd,
        &ws_pkt);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Erro no envio: %s", esp_err_to_name(ret));
    }

    free(message);
}

/*
 * Função pública para enviar mensagens.
 */
esp_err_t websocket_send(const char *message)
{
    if (server == NULL || client_fd < 0)
    {
        return ESP_ERR_INVALID_STATE;
    }

    char *copy = strdup(message);

    if (copy == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    esp_err_t ret = httpd_queue_work(
        server,
        websocket_send_async,
        copy);

    if (ret != ESP_OK)
    {
        free(copy);
    }

    return ret;
}

/*
 * Inicia o servidor WebSocket.
 */
esp_err_t websocket_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Iniciando servidor HTTP...");

    esp_err_t ret = httpd_start(&server, &config);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha ao iniciar servidor");
        return ret;
    }

    httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = websocket_handler,
        .user_ctx = NULL,
        .is_websocket = true};

    ret = httpd_register_uri_handler(server, &ws_uri);
    ESP_LOGI(TAG, "Servidor ativo na porta: %d", config.server_port);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha ao registrar endpoint");

        httpd_stop(server);
        server = NULL;

        return ret;
    }

    ESP_LOGI(TAG, "Servidor WebSocket iniciado!");
    ESP_LOGI(TAG, "Endpoint: /ws");

    return ESP_OK;
}
