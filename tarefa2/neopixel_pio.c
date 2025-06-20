#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "ws2818b.pio.h"

#define LED_COUNT 25
#define LED_PIN 7
#define MIC_CHNL 2
#define MIC_PIN 28

#define ADC_CLOCK_DIV 96.f
#define AMOSTRAS 20


typedef struct {
    uint8_t G, R, B;
} pixel_t;

pixel_t leds[LED_COUNT];
PIO np_pio;
uint sm;


// Define cor de um LED individual METODO BASE
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    if (index < LED_COUNT)
        leds[index] = (pixel_t){g, r, b};
}

// Atualiza todos os LEDs
void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100);
}

// Limpa os LEDs
void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 0, 0, 0);
    npWrite();
}

// acender matriz led COR VERMELHO
void efeito() {
    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 255, 0, 0);
    npWrite();

}

// Lê o microfone com média simples NÃO CONSEGUI FAZER DO MODO SIMPLEs (uint32_t raw = adc_read;)
uint16_t ler_microfone_media() {
    uint32_t soma = 0;
    for (int i = 0; i < AMOSTRAS; i++) {
        adc_run(true);
        uint16_t leitura = adc_fifo_get();
        soma += leitura;
        sleep_us(50);
    }
    return soma / AMOSTRAS;
}

int main() {
    stdio_init_all();
    sleep_ms(2000);  // Tempo para conexão USB

    // Inicializa ADC
    adc_init();
    adc_gpio_init(MIC_PIN);
    adc_select_input(MIC_CHNL);

    adc_fifo_setup(true, true, 1, false, false);
    adc_set_clkdiv(ADC_CLOCK_DIV);

    // Inicializa os LEDs WS2812
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, true);
    ws2818b_program_init(np_pio, sm, offset, LED_PIN, 800000.f);

    npClear();
    //TAMBEM NÃO CONSEGUIR FAZER O CALL BACK FUNCIONAR TEM UM OUTRO ARQUIVO NA PASTA DE COMO SERIA COM CALL BACK
    while (true) {
        
        uint16_t som_medio = ler_microfone_media();
        printf("[Monitoramento] Leitura do microfone: %d\n", som_medio);

        if (som_medio > 2100) {
            printf("[Alerta] Som detectado! Exibindo X pulsante.\n");
            efeito();
        } else {
            printf("[Status] Ambiente silencioso.\n");
            npClear();
        
    }
        sleep_ms(100);
    }

    return 0;
}
