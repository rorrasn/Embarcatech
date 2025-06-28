#include <stdio.h>
#include "pico/stdlib.h"
#include "neopixel_driver.h"
#include "testes_cores.h"
#include "efeitos.h"
#include "efeito_curva_ar.h"
#include "numeros_neopixel.h"
#include <time.h>
#include <stdlib.h>
#include "pico/time.h"  // Garante acesso a time_us_32()

#define botao 5

// Inicializa o sistema e a matriz NeoPixel
void setup() {
    stdio_init_all();
    sleep_ms(1000);  // Aguarda conexão USB (opcional)
    npInit(LED_PIN); // Inicializa matriz NeoPixel
    srand(time_us_32()); // Semente para aleatoriedade
    gpio_init(botao);
    gpio_set_dir(botao, GPIO_IN);
    gpio_pull_up(botao);
    
}

// Sorteia número inteiro entre [min, max]
int sorteia_entre(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// Exibe o número sorteado de 1 a 6
void mostrar_numero_sorteado(int numero) {
    switch (numero) {
        case 1: mostrar_numero_1(); break;
        case 2: mostrar_numero_2(); break;
        case 3: mostrar_numero_3(); break;
        case 4: mostrar_numero_4(); break;
        case 5: mostrar_numero_5(); break;
        case 6: mostrar_numero_6(); break;
    }
}

int main() {
    setup();
    while (1) {
        if (gpio_get(botao) == 0) {
            printf("✅ Botão pressionado!\n");
            
    
    int vezes = sorteia_entre(1, 9);  // Loop entre 1 e 9 execuções
    printf("Mostrando %d números aleatórios...\n", vezes);

    for (int i = 0; i < vezes; i++) {
        int n = sorteia_entre(1, 6);
        printf("Número sorteado: %d ‼ \n", n);
        mostrar_numero_sorteado(n);
        sleep_ms(100);
    }    }
    }
    return 0;
}