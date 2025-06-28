
#include <string.h>
#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "hardware/i2c.h"

#include "font.h"
#include "ssd1306.h" // Use sua lib SSD1306 preferida

#define LED_VERMELHO 13
#define BUZZER 10
#define WIFI_SSID "TP-Link_2D34"
#define WIFI_PASS "83932564"    // Senha da rede Wi-Fi

#define I2C_PORT i2c1 // I2C port (A i2c0 nao funcionou nesse codigo)
#define I2C_SDA 14
#define I2C_SCL 15

#define LARGURA_DA_TELA 128
#define ALTURADA_TELA 64

ssd1306_t ssd; // Instância do display OLED
int dma_chan;

void setup_gpio()
{
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
}

void i2c_init_display()
{
    i2c_init(I2C_PORT, 400 * 1000); // I2C Initialisation. Using it at 400Khz.
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, LARGURA_DA_TELA, ALTURADA_TELA, 0X3C, I2C_PORT); // Inicializa display
    ssd1306_clear(&ssd);
    ssd1306_draw_string(&ssd, 0, 0, 1, "Sistema");
    ssd1306_draw_string(&ssd, 0, 16, 1, "em Repouso"); // Feedback inicial
    ssd1306_show(&ssd);                              // Endereço padrão do SSD1306
    sleep_ms(1000);
}
void modo_alerta(bool alerta)
{
    if (alerta) {
        gpio_put(BUZZER, 1);
        gpio_put(LED_VERMELHO, 1);
        ssd1306_clear(&ssd);
        ssd1306_draw_string(&ssd, 0, 16, 1, "EVACUAR!");
        ssd1306_show(&ssd);
    } else {
        gpio_put(BUZZER, 0);
        gpio_put(LED_VERMELHO, 0);
        ssd1306_clear(&ssd);
        ssd1306_draw_string(&ssd, 0, 0, 1, "Sistema");
    ssd1306_draw_string(&ssd, 0, 16, 1, "em Repouso");
        ssd1306_show(&ssd);
    }
}
#define html_page_rosa "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" \
                      "<!DOCTYPE html><html><head><style>body{background-color: #FFE4E1;}</style></head><body>" \
                      "<h1 style=\"color:red;\">ALERTA!</h1>" \
                      "<p><a style=\"background-color: red; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer;\" href=\"/led/on\">Ligar LED</a></p>" \
                      "<p><a style=\"background-color: red; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer;\" href=\"/led/off\">Desligar LED</a></p>" \
                      "</body></html>\r\n"
                      
#define html_page "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" \
                      "<!DOCTYPE html><html><body>" \
                      "<h1>Controle do LED</h1>" \
                      "<p><a href=\"/led/on\">Ligar LED</a></p>" \
                      "<p><a href=\"/led/off\">Desligar LED</a></p>" \
                      "</body></html>\r\n"

// Callback para lidar com requisições HTTP
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        // Cliente fechou a conexão
        tcp_close(tpcb);
        return ERR_OK;
    }

    // Extrai e interpreta a requisição HTTP
    char *request = (char *)p->payload;

    if (strstr(request, "GET /led/on")) {
        modo_alerta(true);
    } else if (strstr(request, "GET /led/off")) {
        modo_alerta(false);
    }

    // Envia a resposta HTML para o cliente
    tcp_write(tpcb, html_page_rosa, strlen(html_page_rosa), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    // Libera o buffer utilizado
    pbuf_free(p);

    return ERR_OK;
    tcp_close(tpcb);
}

// Callback de conexão: define o callback HTTP para conexões novas
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);
    return ERR_OK;
}

static void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        return;
    }

    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }

    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP rodando na porta 80...\n");
}
int main()
{
    stdio_init_all();
    setup_gpio();
    i2c_init_display();
    sleep_ms(2000); // Aguarda a conexão da porta serial

    // Inicializa o sistema do módulo CYW43439
    if (cyw43_arch_init())
    {
        printf("Erro ao inicializar o módulo CYW43439.\n");
        return 1; // Encerra o programa em caso de erro
    }

    // Ativa o modo "station", que permite se conectar a uma rede existente
    cyw43_arch_enable_sta_mode();

    // Conecta à rede com timeout de 10 segundos
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("Falha ao conectar à rede Wi-Fi.\n");
        return 1; // Encerra se não conseguir conectar
    }
    else
    {
        printf("Conectado à rede Wi-Fi: %s\n", WIFI_SSID); // Exibe o SSID
        // Obtém o endereço IP atual do dispositivo
        uint8_t *ip = (uint8_t *)&cyw43_state.netif[0].ip_addr.addr;
        printf("Para ligar ou desligar o LED, acesse o endereço IP seguido de /led/on ou /led/off\n");
        printf("Endereço IP: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    }
       
    start_http_server();

    // Loop infinito para manter a conexão viva
    while (true)
    {
        cyw43_arch_poll(); // Necessário para manter o funcionamento da pilha de rede
        sleep_ms(1000);    // Aguarda 1 segundo (pode ser ajustado)
    }

    // Encerramento do Wi-Fi (nunca será executado neste código)
    cyw43_arch_deinit();
    return 0;

}
