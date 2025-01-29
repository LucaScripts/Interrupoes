#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "ws2812.pio.h"

// GPIOs
#define BUTTON_A 5
#define BUTTON_B 6
#define WS2812_PIN 7
#define LED_RED 11
#define LED_GREEN 12
#define LED_BLUE 13

// Variáveis globais
volatile int numero_matriz = 0;
const uint LED_COUNT = 25; // 5x5 matriz de LEDs

// Protótipos
void gpio_irq_handler(uint gpio, uint32_t events);
void ws2812_show();
void debounce_and_handle(uint gpio);
void atualiza_matriz(int numero);
void pisca_led_vermelho();

int main() {
    // Configuração dos GPIOs
    stdio_init_all();

    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Configuração da matriz WS2812
    ws2812_init(WS2812_PIN);

    // Configuração das interrupções
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Loop principal
    while (true) {
        pisca_led_vermelho();
    }
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    debounce_and_handle(gpio);
}

void debounce_and_handle(uint gpio) {
    static uint32_t last_time_a = 0;
    static uint32_t last_time_b = 0;
    uint32_t current_time = time_us_32();

    if (gpio == BUTTON_A) {
        if (current_time - last_time_a > 20000) { // 20ms debounce
            numero_matriz = (numero_matriz + 1) % 10;
            atualiza_matriz(numero_matriz);
            last_time_a = current_time;
        }
    } else if (gpio == BUTTON_B) {
        if (current_time - last_time_b > 20000) { // 20ms debounce
            numero_matriz = (numero_matriz - 1 + 10) % 10;
            atualiza_matriz(numero_matriz);
            last_time_b = current_time;
        }
    }
}

void atualiza_matriz(int numero) {
    // Apaga todos os LEDs
    for (int i = 0; i < LED_COUNT; i++) {
        ws2812_set_pixel(i, 0x000000);
    }

    // Configura o padrão de números (substitua com seus próprios padrões)
    // Exemplo simples: apenas iluminar os LEDs superiores para o número
    for (int i = 0; i < numero; i++) {
        ws2812_set_pixel(i, 0x00FF00); // Verde
    }

    ws2812_show();
}

void pisca_led_vermelho() {
    gpio_put(LED_RED, 1);
    sleep_ms(100); // 100ms ligado
    gpio_put(LED_RED, 0);
    sleep_ms(100); // 100ms desligado
} 
