#include "funcao_atividade_.h"
#include "funcoes_neopixel.h" 

int fila[TAM_FILA];
int inicio = 0;
int fim = 0;
int quantidade = 0;
int contador = 0;

absolute_time_t ultimo_toque[NUM_BOTOES];
const uint BOTOES[NUM_BOTOES] = {BOTAO_A, BOTAO_B, BOTAO_JOYSTICK};
const uint LEDS[NUM_BOTOES]   = {LED_VERMELHO, LED_AZUL, LED_VERDE};

volatile bool eventos_pendentes[NUM_BOTOES] = {false, false, false};
volatile bool estado_leds[NUM_BOTOES] = {false, false, false};
volatile bool core1_pronto = false;

void gpio_callback(uint gpio, uint32_t events) {
    for (int i = 0; i < NUM_BOTOES; i++) {
        if (gpio == BOTOES[i] && (events & GPIO_IRQ_EDGE_FALL)) {
            multicore_fifo_push_blocking(i);  // sem desabilitar a interrupção
        }
    }
}

void inicializar_pino(uint pino, uint direcao, bool pull_up, bool pull_down) {
    gpio_init(pino);
    gpio_set_dir(pino, direcao);

    if (direcao == GPIO_IN) {
        if (pull_up) {
            gpio_pull_up(pino);
        } else if (pull_down) {
            gpio_pull_down(pino);
        } else {
            gpio_disable_pulls(pino);
        }
    }
}

void tratar_eventos_leds() {
    core1_pronto = true;

    while (true) {
        uint32_t id1 = multicore_fifo_pop_blocking();  // Aguarda botão pressionado

        sleep_ms(DEBOUNCE_MS);

        if (!gpio_get(BOTOES[id1])) {
            bool outro_pressionado = false;
            for (int i = 0; i < NUM_BOTOES; i++) {
                if (i != id1 && !gpio_get(BOTOES[i])) {
                    outro_pressionado = true;
                    break;
                }
            }
            if (outro_pressionado) {
                while (!gpio_get(BOTOES[id1])) tight_loop_contents();
                continue;
            }

            if (id1 == 0 && index_neo < LED_COUNT) {  // BOTÃO A → incrementa
                uint8_t r = numero_aleatorio(1, 255);
                uint8_t g = numero_aleatorio(1, 255);
                uint8_t b = numero_aleatorio(1, 255);
                npAcendeLED(index_neo, r, g, b);
                index_neo++;

                if (quantidade < TAM_FILA) {
                    fila[fim] = contador++;
                    fim = (fim + 1) % TAM_FILA;
                    quantidade++;
                    imprimir_fila();
                }

            } else if (id1 == 1 && index_neo > 0) {   // BOTÃO B → decrementa
                index_neo--;
                npAcendeLED(index_neo, 0, 0, 0);

                if (quantidade > 0) {
                    int valor = fila[inicio];
                    inicio = (inicio + 1) % TAM_FILA;
                    quantidade--;
                    imprimir_fila();
                }

            /****************************************************************************
             * AO PRESSIONAR O BOTÃO DO JOYSTICK, O SISTEMA ZERA O CONTADOR DE EVENTOS E APAGA TODA A
            MATRIZ DE NEOPIXEL */
            } else if (id1 == 2) {  // <<< BOTÃO JOYSTICK → reset geral
                // Apaga todos os LEDs da matriz NeoPixel
                for (int i = 0; i < LED_COUNT; i++) {
                    npAcendeLED(i, 0, 0, 0);
                    
                }
                index_neo = 0;

                // Zera a fila
                inicio = 0;
                fim = 0;
                quantidade = 0;
                contador = 0;
                imprimir_fila();

                printf("Reset completo do contador!\n");
            }

            // Atualiza LEDs RGB externos
            gpio_put(LED_VERMELHO, (index_neo == LED_COUNT));// Acende LED VERMELHO quando toda matriz NeoPixel estiver acesa
            gpio_put(LED_AZUL,     (index_neo == 0));
            gpio_put(LED_VERDE,    (0));  

            while (!gpio_get(BOTOES[id1])) {
                tight_loop_contents();
            }
        }
    }
}



void imprimir_fila() {
    printf("Fila [tam=%d]: ", quantidade);
    int i = inicio;
    for (int c = 0; c < quantidade; c++) {
        printf("%d ", fila[i]);
        i = (i + 1) % TAM_FILA;
    }
    printf("\n");
}
