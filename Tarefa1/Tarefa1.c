#include <stdio.h>
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

// Pinos
#define vRx 26
#define vRy 27

#define led_r 13
#define led_g 11
#define led_b 12

#define buzzer 21

volatile uint16_t flag_estado = 0;

// Configura joystick
void setup_adc() {
    adc_init();
    adc_gpio_init(vRx);
    adc_gpio_init(vRy);
}

// Configura LEDs e buzzer
void setup_saida() {
    gpio_init(led_r);
    gpio_set_dir(led_r, GPIO_OUT);

    gpio_init(led_g);
    gpio_set_dir(led_g, GPIO_OUT);

    gpio_init(led_b);
    gpio_set_dir(led_b, GPIO_OUT);

    gpio_init(buzzer);
    gpio_set_dir(buzzer, GPIO_OUT);
}
void tocar_buzzer() {
    gpio_put(buzzer, 1);
    sleep_ms(100);
    gpio_put(buzzer, 0);
    sleep_ms(100);
}

void setup() {
    stdio_init_all();
    setup_adc();
    setup_saida();
}

// Lê joystick
uint16_t joystick_read_axis(uint8_t canal) {
    adc_select_input(canal);
    return adc_read();
}
// Timer
int64_t alarme_callback(alarm_id_t id, void *user_data) {
    multicore_fifo_push_blocking(flag_estado);
    return 200000; // Repetir em 2 segundos
}
// Função core 1
void core1_entry() {
    uint16_t  comando_recebido;
    while (1) {
        comando_recebido = multicore_fifo_pop_blocking();
        if (3 == comando_recebido) {
            gpio_put(led_r, 1);
            gpio_put(led_g, 0);
            gpio_put(led_b, 0);
            tocar_buzzer();
        } else if (2 == comando_recebido) {
            gpio_put(led_r, 0);
            gpio_put(led_g, 0);
            gpio_put(led_b, 1);
            
        } else if (1 == comando_recebido) {
            gpio_put(led_r, 0);
            gpio_put(led_g, 1);
            gpio_put(led_b, 0);
            
        } 
    }
}

// Main
int main() {
    setup();
    multicore_launch_core1(core1_entry);
    repeating_timer_t timer;
    add_alarm_in_ms(2000, alarme_callback, NULL, &timer);

    uint16_t valor_x, valor_y;
    while (1) {
        valor_x = joystick_read_axis(0); // Eixo X
        valor_y = joystick_read_axis(1); // Eixo Y
        printf("Eixo X: %d, Eixo Y: %d\n", valor_x, valor_y);
        if (valor_x > 3000) {
            flag_estado = 1; // Ex: eixo X positivo -> Estado 1
        } else if (valor_y > 3000) {
            flag_estado = 2; // Ex: eixo Y positivo -> Estado 2
        } else if (valor_x < 1000 || valor_y < 1000) {
            flag_estado = 3; // Ex: eixo X ou Y negativo -> Estado 3
        } 
        
        sleep_ms(500);
    }
}