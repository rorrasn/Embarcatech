#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>

#define BUZZER_GPIO 21
#define BUZZER_FREQ_HZ 1000

void buzzer_pwm_init() {
    // Configura o GPIO como função PWM
    gpio_set_function(BUZZER_GPIO, GPIO_FUNC_PWM);

    // Descobre o slice PWM que controla esse pino
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_GPIO);

    // Calcula o valor do divisor para obter a frequência desejada
    uint32_t clock_freq = 125000000; // Clock padrão do sistema no Pico
    uint32_t wrap = clock_freq / BUZZER_FREQ_HZ;

    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(BUZZER_GPIO), wrap / 2); // 50% duty

    pwm_set_enabled(slice_num, true);
}

void buzzer_pwm_stop() {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_GPIO);
    pwm_set_enabled(slice_num, false);
    gpio_set_function(BUZZER_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(BUZZER_GPIO, GPIO_OUT);
    gpio_put(BUZZER_GPIO, 0);
}