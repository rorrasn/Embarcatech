#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>

#define BUZZER_PIN 21
#define BUZZER_FREQ_HZ 1000

void buzzer_pwm_init() {
    // Configura o GPIO para PWM
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint channel = pwm_gpio_to_channel(BUZZER_PIN);

    // Frequência do sistema: 125 MHz
    // Vamos usar wrap de 2500 (bom valor) -> frequência = clk_sys / (div * wrap)
    // Para 1kHz, div = 125_000_000 / (1_000 * 2500) = 50.0

    pwm_set_wrap(slice_num, 2500);
    pwm_set_clkdiv(slice_num, 50.0f);  // Divisor ajustado para 1kHz
    pwm_set_chan_level(slice_num, channel, 1250);  // 50% duty cycle

    pwm_set_enabled(slice_num, true);
}

void buzzer_pwm_stop() {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_enabled(slice_num, false);

    gpio_set_function(BUZZER_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);
}
