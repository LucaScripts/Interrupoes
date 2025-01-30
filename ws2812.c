#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "ws2812.pio.h"

// Function declarations
uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | (uint32_t)(b);
}

void put_pixel(PIO pio, uint sm, uint pin, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

// Função para converter hexadecimal para RGB
uint32_t hex_to_rgb(uint32_t hex_value) {
    uint8_t r = (hex_value >> 16) & 0xFF;
    uint8_t g = (hex_value >> 8) & 0xFF;
    uint8_t b = hex_value & 0xFF;
    return urgb_u32(r, g, b);
}

// Definições de pinos
#define LED_RED_PIN 13
#define LED_GREEN_PIN 11
#define LED_BLUE_PIN 12
#define WS2812_PIN 7
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

#define NUM_PIXELS 25
#define DEBOUNCE_TIME_US 200000  // 200ms em microssegundos

volatile int displayed_number = 0;
volatile uint64_t last_press_a = 0;
volatile uint64_t last_press_b = 0;

// Padrões de números para a matriz 5x5
const uint32_t number_patterns[10][25] = {
    // Número 0
    {0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00400000, 0x00000000, 0x00000000, 0x00000000, 0x00400000, 
     0x00400000, 0x00000000, 0x00000000, 0x00000000, 0x00400000, 
     0x00400000, 0x00000000, 0x00000000, 0x00000000, 0x00400000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000},
    // Número 1
    {0x00000000, 0x00000000, 0x00400000, 0x00000000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00000000, 0x00000000, 
     0x00000000, 0x00000000, 0x00400000, 0x00000000, 0x00000000, 
     0x00000000, 0x00000000, 0x00400000, 0x00000000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000},
    // Número 2
    {0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00000000, 0x00000000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000},
    // Número 3
    {0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000},
    // Número 4
    {0x00000000, 0x00400000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000},
    // Número 5
    {0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00000000, 0x00000000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000},
    // Número 6
    {0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00000000, 0x00000000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000},
    // Número 7
    {0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00000000, 0x00400000, 0x00400000, 0x00400000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000},
    // Número 8
    {0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000},
    // Número 9
    {0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00400000, 0x00400000, 0x00000000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000}
};

// Função para exibir número na matriz de LEDs WS2812
void update_ws2812_display(PIO pio, uint sm) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        int index = (4 - (i / 5)) * 5 + (i % 5); // Corrige a orientação dos números
        put_pixel(pio, sm, WS2812_PIN, hex_to_rgb(number_patterns[displayed_number][index]));
    }
}

// Rotina de interrupção para botão A
void button_a_irq_handler(uint gpio, uint32_t events) {
    uint64_t now = time_us_64();
    if (now - last_press_a > DEBOUNCE_TIME_US) {
        last_press_a = now;
        displayed_number = (displayed_number + 1) % 10; // Incrementa o número exibido
        printf("Incremented: %d\n", displayed_number); // Mensagem de depuração
    }
}

// Rotina de interrupção para botão B
void button_b_irq_handler(uint gpio, uint32_t events) {
    uint64_t now = time_us_64();
    if (now - last_press_b > DEBOUNCE_TIME_US) {
        last_press_b = now;
        displayed_number = (displayed_number - 1 + 10) % 10; // Decrementa o número exibido
        printf("Decremented: %d\n", displayed_number); // Mensagem de depuração
    }
}

int main() {
    stdio_init_all();
    printf("Iniciando...\n");

    // Configuração dos LEDs RGB
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);

    // Configuração dos botões
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN); // Adiciona pull-up
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &button_a_irq_handler);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN); // Adiciona pull-up
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &button_b_irq_handler);

    // Configuração da matriz WS2812
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    // Loop principal
    uint32_t last_blink_time = 0;
    while (1) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_blink_time >= 100) { // 100ms para piscar 5 vezes por segundo
            gpio_put(LED_RED_PIN, !gpio_get(LED_RED_PIN));
            last_blink_time = current_time;
        }
        update_ws2812_display(pio, sm);
        sleep_ms(10);
    }
}
