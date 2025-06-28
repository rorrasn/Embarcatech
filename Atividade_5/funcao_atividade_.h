#ifndef FUNCAO_ATIVIDADE_3_H
#define FUNCAO_ATIVIDADE_3_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"   // para __wfi()
#include "hardware/adc.h" // Biblioteca para manipulação do ADC no RP2040
#include "pico/multicore.h"


// ======= DEFINIÇÕES E CONSTANTES =======
#define DEBOUNCE_MS 40
#define DELAY_MS 500

#define BOTAO_A 5
#define BOTAO_B 6
#define BOTAO_JOYSTICK 22

#define LED_VERMELHO 13
#define LED_AZUL 12
#define LED_VERDE 11

#define NUM_BOTOES 3

#define TAM_FILA 25

#define ACAO_INSERIR 1
#define ACAO_REMOVER 2

// ======= PROTÓTIPOS DAS FUNÇÕES =======
void gpio_callback(uint gpio, uint32_t events);
void inicializar_pino(uint pino, uint direcao, bool pull_up, bool pull_down);
void tratar_eventos_leds();
void imprimir_fila();

extern const uint BOTOES[NUM_BOTOES];
extern const uint LEDS[NUM_BOTOES];
extern volatile bool core1_pronto;

extern volatile bool eventos_pendentes[NUM_BOTOES];
extern volatile bool estado_leds[NUM_BOTOES];

#endif