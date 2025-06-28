

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "hardware/timer.h"

#include "setup.h"
#include "tarefa1_temp.h"
#include "tarefa2_display.h"
#include "tarefa3_tendencia.h"
#include "tarefa4_controla_neopixel.h"
#include "neopixel_driver.h"
#include "testes_cores.h"
#include "pico/stdio_usb.h"

// declar as funcoes das tarefas c
void tarefa_3();
void tarefa_5();
void tarefa_4();
void tarefa_2();
void tarefa_1();

// variavelis
float media;
tendencia_t tend;
absolute_time_t ini_tarefa1, fim_tarefa1, tempo1_us, ini_tarefa2, fim_tarefa2, tempo2_us,
    ini_tarefa3, fim_tarefa3, tempo3_us, ini_tarefa4, fim_tarefa4, tempo4_us;


// === Callback da Tarefa (NAO FUNCIONOU DESSA FORMA)===
bool tarefa1_callback(struct repeating_timer *t)
{
       tarefa_1();
       return true;
}
bool tarefa2_callback(struct repeating_timer *t)
{
       tarefa_2();
       return true;
}
bool tarefa3_callback(struct repeating_timer *t)
{
       tarefa_3();
       return true;
}
bool tarefa4_callback(struct repeating_timer *t)
{
       tarefa_4();
       return true;
}

int main()
{
        setup(); // Inicializações: ADC, DMA, interrupções, OLED, etc.
        sleep_ms(2000);

        watchdog_enable(5000, false);

       

        // Inicia o executor de tarefas
        struct repeating_timer timer1, timer2, timer3, timer4;
        add_repeating_timer_ms(1000, tarefa1_callback, NULL, &timer1);
        add_repeating_timer_ms(1500, tarefa2_callback, NULL, &timer2);
        add_repeating_timer_ms(1600, tarefa3_callback, NULL, &timer3);
        add_repeating_timer_ms(1700, tarefa4_callback, NULL, &timer4);
        
        while (true)
        {
         tight_loop_contents(); // Economiza energia, faz ocioso
         watchdog_update();
        }
}

/*******************************/

void tarefa_1()
{
        // --- Tarefa 1: Leitura de temperatura via DMA ---
        ini_tarefa1 = get_absolute_time();
        media = tarefa1_obter_media_temp(&cfg_temp, DMA_TEMP_CHANNEL);
        fim_tarefa1 = get_absolute_time();
        int64_t tempo1_us = absolute_time_diff_us(ini_tarefa1, fim_tarefa1);
}

/*******************************/
void tarefa_2()
{
        // --- Tarefa 3: Análise da tendência térmica ---
        ini_tarefa3 = get_absolute_time();
        tend = tarefa3_analisa_tendencia(media);
        fim_tarefa3 = get_absolute_time();
        int64_t tempo2_us = absolute_time_diff_us(ini_tarefa2, fim_tarefa2);
}
/*******************************/
void tarefa_3()
{
        // --- Tarefa 2: Exibição no OLED ---
        ini_tarefa2 = get_absolute_time();
        tarefa2_exibir_oled(media, tend);
        fim_tarefa2 = get_absolute_time();
        int64_t tempo3_us = absolute_time_diff_us(ini_tarefa3, fim_tarefa3);
}
/*******************************/
void tarefa_4()
{
        // --- Tarefa 4: Cor da matriz NeoPixel por tendência ---
        ini_tarefa4 = get_absolute_time();
        tarefa4_matriz_cor_por_tendencia(tend);
        fim_tarefa4 = get_absolute_time();
        int64_t tempo4_us = absolute_time_diff_us(ini_tarefa4, fim_tarefa4);
}
void tarefa_5()
{
        printf("Temperatura: %.2f °C | T1: %.3fs | T2: %.3fs | T3: %.3fs | T4: %.3fs | Tendência: %s\n",
               media,
               tempo1_us / 1e6,
               tempo2_us / 1e6,
               tempo3_us / 1e6,
               tempo4_us / 1e6,
               tendencia_para_texto(tend));

        // --- Tarefa 5: Extra ---
        if (media < 1)
        {
                npSetAll(COR_BRANCA);
                npWrite();
                sleep_ms(1000);
                npClear();
                npWrite();
        }
}
