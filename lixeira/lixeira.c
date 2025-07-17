#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include <stdlib.h>
#include "hardware/i2c.h"
#include <unistd.h>
#include "pico/binary_info.h"
#include "tof.h"        // time of flight sensor library
#include "lwip/netif.h" // Necessário para netif_default e ip4addr_ntoa

#define I2C_PORT i2c0
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1

#define LED_RED 13                 // Verifique se este é o pino correto para o LED na sua placa
#define WIFI_SSID "VIVO FIBRA-EXT" // Substitua pelo nome da sua rede Wi-Fi
#define WIFI_PASS "19821961aa"     // Substitua pela senha da sua rede Wi-Fi

// Endereço I2C padrão do VL53L0X
#define VL53L0X_DEFAULT_ADDR 0x29

float current_distance_cm;
float MAX_BIN_HEIGHT_CM = 50.0f;
volatile bool trash_bin_is_full = false; // Variável global para o estado da lixeira

#define HTTP_RESPONSE_BUFFER_SIZE 4096
char http_response[HTTP_RESPONSE_BUFFER_SIZE];

// HTML Page content with placeholders for dynamic data
static const char *html_page =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Connection: close\r\n\r\n"
    "<!DOCTYPE html>\r\n"
    "<html lang=\"pt-BR\">\r\n"
    "<head>\r\n"
    "  <meta charset=\"UTF-8\">\r\n"
    "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n"
    "  <meta http-equiv=\"refresh\" content=\"4\">\r\n" // Auto-refresh a cada 4 segundos
    "  <title>Sistema de Controle de Resíduos</title>\r\n"
    "  <style>\r\n"
    "    body {\r\n"
    "      background-color: #C6F4D6;\r\n"
    "      margin: 0;\r\n"
    "      font-family: Arial, sans-serif;\r\n"
    "    }\r\n"
    "\r\n"
    "    h1 {\r\n"
    "      color: #3E8E41;\r\n"
    "      text-align: center;\r\n"
    "      padding: 20px;\r\n"
    "    }\r\n"
    "\r\n"
    "    .grid-container {\r\n"
    "      display: grid;\r\n"
    "      grid-template-columns: 2fr 1fr;\r\n"
    "      gap: 20px;\r\n"
    "      padding: 20px;\r\n"
    "    }\r\n"
    "\r\n"
    "    img {\r\n"
    "      float: right;\r\n"
    "    }\r\n"
    "\r\n"
    "    table {\r\n"
    "      width: 100%%;\r\n"
    "      border-collapse: collapse;\r\n"
    "      background-color: white;\r\n"
    "    }\r\n"
    "\r\n"
    "    th, td {\r\n"
    "      border: 1px solid #3E8E41;\r\n"
    "      padding: 10px;\r\n"
    "      text-align: center;\r\n"
    "      color: #3E8E41;\r\n"
    "    }\r\n"
    "\r\n"
    "    th {\r\n"
    "      background-color: #A6E2B6;\r\n"
    "    }\r\n"
    "\r\n"
    "    @media (max-width: 768px) {\r\n"
    "      .grid-container {\r\n"
    "        grid-template-columns: 1fr;\r\n"
    "      }\r\n"
    "    }\r\n"
    "    .full-status { color: red; font-weight: bold; }\r\n"
    "    .empty-status { color: green; font-weight: bold; }\r\n"
    "  </style>\r\n"
    "</head>\r\n"
    "\r\n"
    "<body>\r\n"
    "  <h1>🟩 🗑🗑 Sistema de Controle de Resíduos 🗑🗑 🟩 </h1>\r\n"
    "\r\n"
    "  <div class=\"grid-container\">\r\n"
    "         <div class=\"map\">\r\n"
    "    <iframe width=\"100%%\" height=\"450\" src=\"https://maps.google.com/maps?q=-2.5286459,-44.2952611&z=14&output=embed\"></iframe>\r\n"
    "  </div>\r\n"
    "    <div>\r\n"
    "      <table>\r\n"
    "        <tr>\r\n"
    "          <th>LOCAL</th>\r\n"
    "          <th>PAPEL</th>\r\n"
    "          <th>PLÁSTICO</th>\r\n"
    "          <th>METAL</th>\r\n"
    "          <th>VIDRO</th>\r\n"
    "        </tr>\r\n"
    "        <tr>\r\n"
    "          <td>CENTRO</td>\r\n"
    "          <td>%.2f cm</td>\r\n" // Distância do sensor
    "          <td>%.0f%%</td>\r\n"  // Porcentagem de volume
    "          <td>100%%</td>\r\n"   // Exemplo fixo
    "          <td>25%%</td>\r\n"    // Exemplo fixo
    "        </tr>\r\n"
    "        <tr>\r\n"
    "          <td>MONTE C</td>\r\n"
    "          <td>10%%</td>\r\n"
    "          <td>10%%</td>\r\n"
    "          <td>50%%</td>\r\n"
    "          <td>40%%</td>\r\n"
    "        </tr>\r\n"
    "        <tr>\r\n"
    "          <td>ANJO G</td>\r\n"
    "          <td>10%%</td>\r\n"
    "          <td>11%%</td>\r\n"
    "          <td>150%%</td>\r\n"
    "          <td>50%%</td>\r\n"
    "        </tr>\r\n"
    "        <tr>\r\n"
    "          <td>CALHAU</td>\r\n"
    "          <td>10%%</td>\r\n"
    "          <td>11%%</td>\r\n"
    "          <td>20%%</td>\r\n"
    "          <td>50%%</td>\r\n"
    "        </tr>\r\n"
    "      </table>\r\n"
    "      <br>\r\n"
    "      <div>\r\n"
    "        <img src=\"https://horizonteambiental.com.br/wp-content/uploads/2023/01/Cestos-coletores-de-residuos-o-que-e-cada-cor.jpg\" width=\"300\" height=\"100\" alt=\"Imagem do logo da lixeira\">\r\n"
    "        <p><span class=\"%s\">%s</span></p>\r\n" // Para o estado Cheio/Vazio
    "      </div>\r\n"
    "    </div>\r\n"
    "  </div>\r\n"
    "\r\n"
    "</body>\r\n"
    "</html>\r\n";

// Configura pinos GPIO (LEDs e XSHUTs)
void setup_pins()
{
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_put(LED_RED, 0);
}

// Configura o barramento I2C
void configura_i2c()
{
    i2c_init(I2C_PORT, 400 * 1000); // Usando I2C_PORT (i2c0) a 400kHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN); // Habilita pull-ups internos para SDA
    gpio_pull_up(I2C_SCL_PIN); // Habilita pull-ups internos para SCL
}

// Função para medir distância com o sensor VL53L0X
float measure_distance()
{
    int dist_mm = tofReadDistance();
    if (dist_mm < 0)
    {
        printf("Erro na leitura do VL53L0X. Retornando altura máxima.\n");
        return MAX_BIN_HEIGHT_CM; // Retorna altura máxima em caso de erro
    }
    else
    {
        return dist_mm / 10.0f; // Converte milímetros para centímetros
    }
}

// Calcula a porcentagem de volume da lixeira
float calcular_volume_percentual(float distancia_cm, float max_altura_cm)
{
    // A lógica é:
    // - Se a distância for 0 (sensor tocando o lixo), está 100% cheio.
    // - Se a distância for igual ou maior que a altura máxima da lixeira, está 0% cheio.
    // - Caso contrário, calcula a porcentagem com base no espaço ocupado.
    if (distancia_cm <= 0)
    {
        return 100.0f; // Completamente cheio ou sensor muito próximo
    }
    else if (distancia_cm >= max_altura_cm)
    {
        return 0.0f; // Totalmente vazio ou além da altura máxima
    }
    else
    {
        // Calcula a porcentagem de preenchimento
        return ((max_altura_cm - distancia_cm) / max_altura_cm) * 100.0f;
    }
}

// Callback para lidar com requisições HTTP
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p == NULL)
    {
        // Cliente fechou a conexão ou erro no pbuf
        tcp_close(tpcb);
        return ERR_OK;
    }
    // Atualiza distância e estado da lixeira no momento da requisição
    current_distance_cm = measure_distance();
    float volume_percent = calcular_volume_percentual(current_distance_cm, MAX_BIN_HEIGHT_CM);
    trash_bin_is_full = volume_percent >= 80.0f;

    const char *status_class = trash_bin_is_full ? "full-status" : "empty-status";
    const char *status_text = trash_bin_is_full ? "ATENCÃO alguma lixeira esta CHEIA!" : "Sistema normal";

    // Prepara a resposta HTTP com os dados dinâmicos
    snprintf(http_response, HTTP_RESPONSE_BUFFER_SIZE, html_page,
             current_distance_cm, volume_percent, status_class, status_text);

    // Envia o conteúdo do buffer http_response para o cliente
    tcp_write(tpcb, http_response, strlen(http_response), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb); // Garante que os dados sejam enviados imediatamente

    // Libera o buffer pbuf
    pbuf_free(p);
    tcp_close(tpcb);
    return ERR_OK;
}

// Callback de conexão: define o callback HTTP para conexões novas
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    if (err != ERR_OK || newpcb == NULL)
    {
        printf("Erro ao aceitar nova conexão: %d\n", err);
        if (newpcb)
            tcp_abort(newpcb);
        return err;
    }
    tcp_recv(newpcb, http_callback); // Registra o callback para receber dados HTTP
    return ERR_OK;
}

// Inicia o servidor HTTP na porta 80
static void start_http_server(void)
{
    struct tcp_pcb *pcb = tcp_new(); // Cria um novo bloco de controle de protocolo (PCB)
    if (!pcb)
    {
        printf("Erro ao criar PCB para o servidor HTTP!\n");
        return;
    }

    // Liga o PCB à porta 80 (HTTP)
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Erro ao ligar o servidor na porta 80!\n");
        tcp_abort(pcb); // Aborta se falhar no bind
        return;
    }

    // Coloca o PCB em modo de escuta
    pcb = tcp_listen(pcb);
    if (!pcb)
    {
        printf("Erro ao iniciar a escuta do servidor na porta 80!\n");
        tcp_abort(pcb); // Aborta se falhar no listen
        return;
    }
    tcp_accept(pcb, connection_callback); // Registra o callback para aceitar novas conexões
    printf("Servidor HTTP rodando na porta 80...\n");
}

int main()
{
    stdio_init_all(); // Inicializa comunicação serial (USB CDC ou UART)
    setup_pins();     // Configura pinos GPIO
    configura_i2c();  // Configura barramento I2C
    sleep_ms(2000);   // Aguarda um tempo para a serial iniciar
    printf("Iniciando Sistema de Lixeira Inteligente...\n");

    // Inicializa o sistema do módulo CYW43439 (Wi-Fi)
    if (cyw43_arch_init())
    {
        printf("Erro ao inicializar o Wifi.\n");
        return 1; // Encerra o programa em caso de erro
    }
    // Ativa o modo "station", que permite se conectar a uma rede existente
    cyw43_arch_enable_sta_mode();

    /* Configure IP fixo
    ip4_addr_t ip, gw, mask;
    IP4_ADDR(&ip, 192, 168, 15, 150);   // IP fixo desejado
    IP4_ADDR(&gw, 192, 168, 15, 1);     // Gateway (normalmente IP do roteador)
    IP4_ADDR(&mask, 255, 255, 255, 0); // Máscara de sub-rede

    struct netif *iface = netif_default;
    if (iface == NULL){
        printf("Erro: interface de rede não está pronta.\n");
    }
    else{
        dhcp_stop(iface);                       // Desativa DHCP
        netif_set_addr(iface, &ip, &mask, &gw); // Define IP, máscara e gateway
    }*/

    // Conecta à rede Wi-Fi com timeout de 15 segundos
    printf("Conectando... à rede Wi-Fi: %s...\n", WIFI_SSID);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 15000))
    {
        printf("Falha ao conectar à rede Wi-Fi. Verifique SSID e senha.\n");
        // Em caso de falha, você pode tentar reconectar ou simplesmente retornar 1
        return 1;
    }
    else
    {
        // Obtém e imprime o endereço IP de forma robusta
        if (netif_default != NULL && netif_is_up(netif_default) && !ip4_addr_isany_val(netif_default->ip_addr))
        {
            printf("Endereço IP: %s\n", ip4addr_ntoa(&netif_default->ip_addr));
        }
        else
        {
            printf("Wi-Fi Conectado, mas IP ainda não disponível. Aguarde e tente acessar.\n");
        }
    }

    // Inicializa o sensor VL53L0X ;
    tofInit(0, 0x29, 0);
    start_http_server();

    // Loop principal: mede a distância e atualiza o estado do LED
    while (true)
    {
        // Permite que a pilha de rede LWIP processe eventos e mantém a conexão Wi-Fi ativa
        cyw43_arch_poll();
        sleep_ms(50); // Pequeno atraso para não saturar a CPU

        current_distance_cm = measure_distance(); // Atualiza a distância global
        float volume_percent = calcular_volume_percentual(current_distance_cm, MAX_BIN_HEIGHT_CM);

        printf("Distancia: %.2f cm | Volume: %.2f%% cheio\n", current_distance_cm, volume_percent);

        // Atualiza o estado da lixeira (cheia/vazia) e o LED de acordo
        if (volume_percent >= 80.0f){ // Lixeira é considerada "cheia" a partir de 80%
            trash_bin_is_full = true;
            gpio_put(LED_RED, 1); // Acende o LED
        }
        else{
            trash_bin_is_full = false;
            gpio_put(LED_RED, 0); // Apaga o LED
        }

        time_us_64();
    }

    cyw43_arch_deinit(); // Esta linha não será atingida em um loop infinito, mas é boa prática
    return 0;
}