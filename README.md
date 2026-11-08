# ESP32 WebSocket – Monitoramento de Sensor

Projeto de monitoramento em tempo real utilizando um ESP32 como servidor WebSocket. O dispositivo realiza leituras de um sensor e transmite os valores para uma página web por meio de uma conexão Wi-Fi.

Atualmente, o sensor utiliza valores simulados para validar a comunicação. A estrutura permite substituir essa simulação pela leitura de um LDR utilizando o ADC do ESP32.

---

## 1. Tecnologias utilizadas

| Tecnologia | Finalidade |
|---|---|
| ESP32-WROOM-32D | Microcontrolador |
| ESP-IDF 5.5.6 | Framework de desenvolvimento |
| FreeRTOS | Gerenciamento de tarefas |
| Wi-Fi | Comunicação em rede |
| HTTP Server | Implementação do servidor WebSocket |
| WebSocket | Comunicação bidirecional em tempo real |
| C | Linguagem utilizada no firmware |
| HTML, CSS e JavaScript | Interface de monitoramento |

## 2. Estrutura do projeto

O firmware foi organizado em componentes independentes, cada um responsável por uma funcionalidade específica.

```text
ws_server/
├── main/
│   ├── main.c
│   └── CMakeLists.txt
│
├── components/
│   ├── wifi/
│   │   ├── include/wifi.h
│   │   ├── wifi.c
│   │   └── CMakeLists.txt
│   │
│   ├── websocket/
│   │   ├── include/websocket.h
│   │   ├── websocket.c
│   │   └── CMakeLists.txt
│   │
│   └── sensor/
│       ├── include/sensor.h
│       ├── sensor.c
│       └── CMakeLists.txt
│
├── CMakeLists.txt
└── sdkconfig
```

### Responsabilidades dos componentes

- **wifi:** inicialização da interface de rede, conexão e obtenção do endereço IP.
- **websocket:** inicialização do servidor, registro do endpoint, recebimento e envio de mensagens.
- **sensor:** fornece os valores de leitura. Atualmente, utiliza uma simulação.
- **main:** inicializa os componentes e coordena o envio periódico das leituras.

## 3. Pré-requisitos

Antes de iniciar, instale e configure:

1. [Visual Studio Code](https://code.visualstudio.com/).
2. Extensão [ESP-IDF para VS Code](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension).
3. ESP-IDF versão 5.5.6 ou compatível.
4. Drivers USB da placa ESP32, quando necessários.
5. Uma placa ESP32 conectada ao computador.
6. Um navegador com suporte a WebSocket.

O ESP32-WROOM-32D utiliza redes Wi-Fi de 2,4 GHz.

## 4. Configuração do projeto

### 4.1. Credenciais da rede Wi-Fi

Abra o arquivo:

```text
components/wifi/wifi.c
```

Localize as definições `WIFI_SSID` e `WIFI_PASSWORD` e preencha os valores com o nome e a senha da rede Wi-Fi que será utilizada.

Essas informações serão utilizadas pelo componente Wi-Fi durante a inicialização.

### 4.2. Configuração do WebSocket

O servidor disponibiliza o endpoint `/ws`, utilizado pelos clientes para estabelecer a comunicação.

A configuração está localizada em:

```text
components/websocket/websocket.c
```

O registro do endpoint utiliza:

```c
httpd_uri_t ws_uri = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = websocket_handler,
    .user_ctx = NULL,
    .is_websocket = true
};
```

O suporte a WebSocket também precisa estar habilitado no projeto.

Para verificar, abra a configuração do ESP-IDF com:

```powershell
idf.py menuconfig
```

Procure a opção de suporte a WebSocket do HTTP Server e verifique se está habilitada. A configuração também pode ser conferida diretamente no arquivo `sdkconfig`:

```ini
CONFIG_HTTPD_WS_SUPPORT=y
```

## 5. Compilação e gravação do firmware

Com o projeto aberto no VS Code, inicialize o ambiente ESP-IDF pelo terminal da extensão.

### 5.1. Selecionar o alvo

Execute:

```powershell
idf.py set-target esp32
```

Esse comando configura o projeto para a arquitetura ESP32.

### 5.2. Compilar

Execute:

```powershell
idf.py build
```

O ESP-IDF irá compilar os componentes e gerar o firmware.

Caso existam erros de compilação, verifique os arquivos indicados no terminal.

### 5.3. Gravar na placa

Conecte o ESP32 ao computador e identifique a porta serial utilizada.

Execute, substituindo `COMx` pela porta correspondente:

```powershell
idf.py -p COMx flash
```

Para gravar o firmware e abrir o monitor serial em seguida:

```powershell
idf.py -p COMx flash monitor
```

## 6. Identificação do endereço IP

Após a inicialização, o ESP32 irá se conectar à rede Wi-Fi.

O componente Wi-Fi apresenta o endereço IP obtido no monitor serial.

Exemplo de mensagem:

```text
I (1234) WIFI: IP obtido: [ENDEREÇO IP]
```

Copie o endereço apresentado. Ele será utilizado posteriormente na configuração da página HTML.

Caso o roteador atribua um endereço IP diferente após uma nova conexão, será necessário atualizar o endereço na página.

Para evitar alterações frequentes, é possível configurar uma reserva de endereço IP no roteador.

## 7. Configuração e execução da página web

### 7.1. Configuração do endereço IP

Abra o arquivo HTML responsável pela interface de monitoramento.

Localize a criação da conexão WebSocket:

```javascript
ws = new WebSocket("ws://IP_DO_ESP32/ws");
```

Substitua `IP_DO_ESP32` pelo endereço IP identificado na etapa anterior.

O endereço deve corresponder ao IP atual do dispositivo na rede local.

### 7.2. Executando a interface

Após configurar o endereço IP:

1. Abra o arquivo HTML no navegador.
2. Verifique se o computador e o ESP32 estão conectados à mesma rede local.
3. Clique no botão **Conectar ao ESP32**.
4. Aguarde a confirmação de conexão na interface.

A página estabelece uma conexão com:

```text
ws://IP_DO_ESP32/ws
```

Quando a conexão é estabelecida, o navegador envia uma mensagem inicial:

```javascript
ws.send("conectado");
```

Essa mensagem permite que o ESP32 registre o descritor do cliente conectado.

A partir desse momento, o firmware pode enviar as leituras do sensor para o navegador.

**Observação:** o servidor WebSocket não hospeda a página HTML. A interface é aberta separadamente no navegador.

## 8. Funcionamento do firmware

A inicialização e o funcionamento contínuo são coordenados pelo arquivo `main.c`.

A sequência de execução é:

1. Inicialização da memória NVS.
2. Inicialização e conexão Wi-Fi.
3. Inicialização do servidor WebSocket.
4. Leitura do sensor.
5. Conversão do valor para uma mensagem de texto.
6. Envio da mensagem ao cliente conectado.
7. Espera pelo próximo ciclo de leitura.

O envio periódico é controlado por:

```c
vTaskDelay(pdMS_TO_TICKS(100));
```

O valor representa um intervalo de 100 milissegundos, correspondendo a aproximadamente 10 ciclos de leitura por segundo.

### Principais funções

| Função | Componente | Responsabilidade |
|---|---|---|
| `wifi_init()` | Wi-Fi | Inicializa a conexão de rede |
| `websocket_start()` | WebSocket | Inicializa o servidor |
| `websocket_send()` | WebSocket | Envia uma mensagem ao cliente |
| `websocket_handler()` | WebSocket | Recebe e responde mensagens |
| `sensor_read()` | Sensor | Retorna o valor da leitura |

## 9. Fluxo de comunicação

```text
                 ESP32
                   |
             Inicialização
                   |
              Conexão Wi-Fi
                   |
           Servidor WebSocket
                   |
              sensor_read()
                   |
          Conversão para texto
                   |
             websocket_send()
                   |
             Rede Wi-Fi
                   |
             Navegador
                   |
             onmessage()
                   |
          Atualização da interface
```

## 10. Limitações e melhorias futuras

A implementação atual possui algumas limitações:

- O sensor utiliza valores simulados, ainda sem leitura física do LDR.
- O servidor registra apenas um cliente para envio de mensagens.
- O registro do cliente ocorre após o recebimento da primeira mensagem.
- O endereço IP precisa ser configurado manualmente na página HTML.
- A frequência de envio pode ser ajustada conforme a necessidade da aplicação.

Como melhorias futuras, estão previstas a implementação da leitura real do LDR pelo ADC, o tratamento de desconexões e reconexões, o suporte a múltiplos clientes e a exibição de gráficos com o histórico das leituras.

## 11. Inicialização do projeto

Para executar o projeto pela primeira vez, configure as credenciais Wi-Fi e compile o firmware. Depois de gravá-lo no ESP32, abra o monitor serial para acompanhar a inicialização e identificar o endereço IP obtido.

Com o endereço em mãos, atualize a configuração da página HTML e abra o arquivo no navegador. Certifique-se de que o computador e o ESP32 estejam conectados à mesma rede Wi-Fi e clique no botão de conexão.

Se tudo estiver configurado corretamente, os valores enviados pelo ESP32 começarão a aparecer na página.