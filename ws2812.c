#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

// Variáveis globais para controle do LED e cor
uint8_t selected_led = 0; // Índice do LED a ser controlado (0 a 24)
uint8_t selected_r = 0;   // Intensidade do vermelho (0 a 255)
uint8_t selected_g = 0;   // Intensidade do verde (0 a 255)
uint8_t selected_b = 80;   // Intensidade do azul (0 a 255)

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

uint32_t led_buffer[NUM_PIXELS] = {0}; // Buffer para armazenar as cores de todos os LEDs

void update_led_buffer() {
    // Apaga todos os LEDs
    for (int i = 0; i < NUM_PIXELS; i++) {
        led_buffer[i] = 0; // Desliga todos os LEDs
    }

    // Configura a cor para o LED selecionado
    led_buffer[selected_led] = urgb_u32(selected_r, selected_g, selected_b);
}

void set_leds_from_buffer() {
    // Envia o estado de todos os LEDs para a matriz
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(led_buffer[i]);
    }
}

int main() {
    stdio_init_all();
    printf("WS2812 5x5 Matrix - Single LED Control\n");

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    while (1) {
        // Atualiza o buffer com o LED e a cor selecionados
        update_led_buffer();

        // Envia o estado do buffer para a matriz
        set_leds_from_buffer();

        // Simula alteração de LED e cor para testes
        sleep_ms(100); // Espera 1 segundo
        selected_led = (selected_led + 1) % NUM_PIXELS; // Muda para o próximo LED

    }

    return 0;
}
