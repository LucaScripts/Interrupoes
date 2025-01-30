#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "ws2812.pio.h"

// Declarações de funções
uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    // Reduz a intensidade dos LEDs dividindo os valores RGB por 4
    r = r / 4;
    g = g / 4;
    b = b / 4;
    return ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | (uint32_t)(b);
}

void colocar_pixel(PIO pio, uint sm, uint pin, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

// Função para converter hexadecimal para RGB
uint32_t hex_para_rgb(uint32_t valor_hex) {
    uint8_t r = (valor_hex >> 16) & 0xFF;
    uint8_t g = (valor_hex >> 8) & 0xFF;
    uint8_t b = valor_hex & 0xFF;
    return urgb_u32(r, g, b);
}

// Definições de pinos
#define PINO_LED_VERMELHO 13
#define PINO_LED_VERDE 11
#define PINO_LED_AZUL 12
#define PINO_WS2812 7
#define PINO_BOTAO_A 5
#define PINO_BOTAO_B 6

#define NUM_PIXELS 25
#define TEMPO_DEBOUNCE_US 200000  // 200ms em microssegundos

volatile int numero_exibido = 0;
volatile uint64_t ultima_pressao_a = 0;
volatile uint64_t ultima_pressao_b = 0;

// Padrões de números para a matriz 5x5
const uint32_t padroes_numeros[10][25] = {
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
     0x00000000, 0x00400000, 0x00000000, 0x00000000, 0x00000000},
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
     0x00400000, 0x00400000, 0x00400000 ,0x00000000, 0x00000000, 
     0x00000000, 0x00000000, 0x00000000, 0x00400000, 0x00000000, 
     0x00000000, 0x00400000, 0x00000000, 0x00000000, 0x00000000},
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
     0x00000000, 0x00400000, 0x00000000, 0x00000000, 0x00000000}
};

// Função para exibir número na matriz de LEDs WS2812
void atualizar_display_ws2812(PIO pio, uint sm) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        int index = (4 - (i / 5)) * 5 + (i % 5); // Verifique se a indexação está correta
        uint32_t cor = hex_para_rgb(padroes_numeros[numero_exibido][index]);
        colocar_pixel(pio, sm, PINO_WS2812, cor);
    }
}

// Manipulador de interrupção para os botões
void manipulador_irq_gpio(uint gpio, uint32_t eventos) {
    uint64_t agora = time_us_64();

    // Verifica se o botão A foi pressionado e se passou o tempo de debounce
    if (gpio == PINO_BOTAO_A && agora - ultima_pressao_a > TEMPO_DEBOUNCE_US) {
        ultima_pressao_a = agora;
        numero_exibido = (numero_exibido + 1) % 10; // Incrementa o número exibido
        printf("Incrementado: %d\n", numero_exibido);
    } 
    
    // Verifica se o botão B foi pressionado e se passou o tempo de debounce
    if (gpio == PINO_BOTAO_B && agora - ultima_pressao_b > TEMPO_DEBOUNCE_US) {
        ultima_pressao_b = agora;
        numero_exibido = (numero_exibido - 1 + 10) % 10; // Decrementa o número exibido
        printf("Decrementado: %d\n", numero_exibido);
    }

    atualizar_display_ws2812(pio0, 0); // Atualiza a matriz imediatamente
}

int main() {
    stdio_init_all(); // Inicializa a entrada e saída padrão
    printf("Iniciando...\n");

    // Configuração dos LEDs RGB
    gpio_init(PINO_LED_VERMELHO);
    gpio_set_dir(PINO_LED_VERMELHO, GPIO_OUT);
    gpio_init(PINO_LED_VERDE);
    gpio_set_dir(PINO_LED_VERDE, GPIO_OUT);
    gpio_init(PINO_LED_AZUL);
    gpio_set_dir(PINO_LED_AZUL, GPIO_OUT);

    // Configuração dos botões
    gpio_init(PINO_BOTAO_A);
    gpio_set_dir(PINO_BOTAO_A, GPIO_IN);
    gpio_pull_up(PINO_BOTAO_A); // Adiciona pull-up
    gpio_set_irq_enabled_with_callback(PINO_BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &manipulador_irq_gpio);

    gpio_init(PINO_BOTAO_B);
    gpio_set_dir(PINO_BOTAO_B, GPIO_IN);
    gpio_pull_up(PINO_BOTAO_B); // Adiciona pull-up
    gpio_set_irq_enabled_with_callback(PINO_BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &manipulador_irq_gpio);

    // Configuração da matriz WS2812
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, PINO_WS2812, 800000, false);

    // Loop principal
    uint32_t ultima_vez_piscar = 0;
    while (1) {
        uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
        if (tempo_atual - ultima_vez_piscar >= 100) { // 100ms para piscar 5 vezes por segundo
            gpio_put(PINO_LED_VERMELHO, !gpio_get(PINO_LED_VERMELHO));
            ultima_vez_piscar = tempo_atual;
        }
        atualizar_display_ws2812(pio, sm);
        sleep_ms(10);
    }
}
