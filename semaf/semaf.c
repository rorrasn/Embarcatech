#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"


// Pinos
#define LED_VERMELHO 13
#define LED_VERDE 11
#define BUZZER 10
#define BOTAO_A 5
#define BOTAO_B 6

volatile bool botao_pressionado = false;

//inicia IO
void setup_all(){
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_init(BOTAO_A);//botao A
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B);//botao B
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
}
bool esperar_com_interrupcao(int tempo_ms) {
    const int intervalo = 500;
    int passado = 0;
    while (passado < tempo_ms) {
        if (botao_pressionado) return false;
        sleep_ms(intervalo);
        passado += intervalo;
    }
    return true;
}

// Cronômetro regressivo interrompível (últimos 5 segundos)
bool cronometro_regressivo_interrompivel(int total_segundos) {
    int espera = (total_segundos - 5000);
    if (!esperar_com_interrupcao(espera)) return false;

    for (int i = 5; i > 0; i--) {
        if (botao_pressionado) return false;
        printf("%d\n", i);
        sleep_ms(1000);
    }
    return true;
}

bool cor_vermelha() {
    printf("Sinal VERMELHO\n");
    gpio_put(LED_VERMELHO, 1);
    gpio_put(BUZZER, 1);
    bool completou = cronometro_regressivo_interrompivel(10000);
    gpio_put(BUZZER, 0);
    gpio_put(LED_VERMELHO, 0);
    return completou;
}

bool cor_verde() {
    printf("Sinal VERDE\n");
    gpio_put(LED_VERDE, 1);
    bool completou = esperar_com_interrupcao(10000); 
    gpio_put(LED_VERDE, 0);
    return completou;
}

void cor_amarela() {
    printf("Sinal AMARELO\n");
    gpio_put(LED_VERMELHO, 1);
    gpio_put(LED_VERDE, 1);  
    for (int i = 0; i < 3; i++) {
            gpio_put(BUZZER, 1);
            sleep_ms(500);
            gpio_put(BUZZER, 0);
            sleep_ms(500);        
    }
    gpio_put(LED_VERMELHO, 0);
    gpio_put(LED_VERDE, 0);
}
// interrupção
void isr_botoes(uint gpio, uint32_t events) {
       botao_pressionado = true;
}
// Main
int main() {
    stdio_init_all();
    setup_all();
    sleep_ms(2000);

    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &isr_botoes);
    gpio_set_irq_enabled(BOTAO_B, GPIO_IRQ_EDGE_FALL, true);

    while (true) {
        cor_vermelha();
        if (botao_pressionado) goto interrupcao;
        cor_verde();
        if (botao_pressionado) goto interrupcao;
        cor_amarela();
        if (botao_pressionado) goto interrupcao;
        continue;

    interrupcao:
        cor_amarela();
        botao_pressionado = false;
    }
    tight_loop_contents();
}