#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "blink.pio.h"

#define NEOPIXEL_PIN 7
#define MICROPHONE 28
#define CHANNEL 2
const int LED_COUNT = 25;



PIO pio = pio0;
uint sm;

void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | (uint32_t)(b);
}

void show_color(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = urgb_u32(r, g, b);
    for (int i = 0; i < LED_COUNT; i++) {
        put_pixel(color);
    }
}

// Apaga todos os LEDs
void clear_leds(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = urgb_u32(r, g, b);
    for (int i = 0; i < LED_COUNT; i++) {
        put_pixel(color);
    }
}


int main() {
    stdio_init_all();
    sleep_ms(2000);

    adc_init();
    adc_gpio_init(MICROPHONE);
    adc_select_input(CHANNEL);
    

    uint offset = pio_add_program(pio, &blink_program);
    sm = pio_claim_unused_sm(pio, true);
    blink_program_init(pio, sm, offset, NEOPIXEL_PIN, 800000);
    
    clear_leds(0, 0, 0);
    printf("Sistema iniciado...\n");
    

    while (true) {
        sleep_ms(200);  // Tempo de atualização
        
        uint16_t som = adc_read();  // Leitura direta
        sleep_ms(200);   
        
        printf("Som: %d\n", som);
        if (som > 2200) {
            printf("[Status] Ambiente com som.\n");
            show_color(255, 255, 255);  // acende se som detectado
        } else {
            printf("[Status] Ambiente silencioso.\n");
            clear_leds(0, 0, 0);           // Apaga se silencioso
        }
        return true;
}
    
    
}