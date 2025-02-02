// Código para a tarefa 27/01
// Aluna: Maryana Souza Silveira
// Este código apresenta um led vermelho piscando 5 vezes por segundo e um contador de 0 a 9 que é exibido na matriz
// de LEDs WS2812. Ao clicar no botão A, a contagem é incrementada e ao clicar no botão B, a contagem é decrementada.

#include <stdio.h> // Biblioteca para entrada e saída padrão
#include <stdlib.h> // Biblioteca padrão do C
#include "pico/stdlib.h" // Biblioteca do Pico para funções padrão
#include "hardware/pio.h" // Biblioteca para controle de PIO
#include "hardware/clocks.h" // Biblioteca para controle de clock
#include "Tarefa2701.pio.h" // Código do programa para controle de LEDs WS2812

#define numLeds 25 // Número de LEDs WS2812
#define matriz_leds 7 // Pino de controle dos LEDs WS2812

#define LED_RED 13 // Pino do LED vermelho RGB

const uint buttonA = 5; // Pino do botão A - acrescenta contagem
const uint buttonB = 6; // Pino do botão B - diminui contagem

int contagem = 0; // Variável para armazenar a contagem

// Variável global para armazenar a cor (Entre 0 e 255 para intensidade)
uint8_t led_r = 0; // Intensidade do vermelho
uint8_t led_g = 100; // Intensidade do verde
uint8_t led_b = 0; // Intensidade do azul

// Declaração das funções a serem utilizadas
void gpio_irq_handler(uint gpio, uint32_t events); // Função de interrupção
static inline void put_pixel(uint32_t pixel_grb); // Função para atualizar um LED
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b); // Função para converter RGB para uint32_t
void printnum(uint8_t r, uint8_t g, uint8_t b, int contagem); // Função para imprimir o número da contagem


int main()
{
    stdio_init_all(); // Inicializa a comunicação serial
    // Inicializa o PIO
    PIO pio = pio0; 
    int sm = 0; 
    uint offset = pio_add_program(pio, &ws2812_program);

    gpio_init(LED_RED); // Inicializa o pino do LED vermelho
    gpio_set_dir(LED_RED, GPIO_OUT); // Define o pino do LED vermelho como saída

    gpio_init(buttonA); // Inicializa o pino do botão A
    gpio_set_dir(buttonA, GPIO_IN); // Define o pino do botão A como entrada
    gpio_pull_up(buttonA); // Habilita o pull-up no pino do botão A

    gpio_init(buttonB); // Inicializa o pino do botão B
    gpio_set_dir(buttonB, GPIO_IN); // Define o pino do botão B como entrada
    gpio_pull_up(buttonB); // Habilita o pull-up no pino do botão B

    ws2812_program_init(pio, sm, offset, matriz_leds, 800000, false); // Inicializa o programa para controle dos LEDs WS2812

    gpio_set_irq_enabled_with_callback(buttonA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilita a interrupção no botão A
    gpio_set_irq_enabled_with_callback(buttonB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilita a interrupção no botão B

    printnum(led_r, led_g, led_b, contagem); // Imprime o número 0 na contagem

    while (1) // Loop para acender o led vermelho do RGB 5 vezes por segundo
    {
        gpio_put(LED_RED,1);
        sleep_ms(100);
        gpio_put(LED_RED,0);
        sleep_ms(100);
    }

    return 0;
}

// Função de interrupção
void gpio_irq_handler(uint gpio, uint32_t events) {
    static uint32_t last_interrupt_time = 0;  // Variável para armazenar o último tempo de interrupção
    uint32_t current_time = get_absolute_time(); // Variável para armazenar o tempo atual
    
    // Verifica se o tempo entre interrupções é maior que 200ms
    if (current_time - last_interrupt_time > 200000) {  // 200ms de debounce
        if (gpio == buttonA) { // Caso a interrupção seja no botão A
            if(contagem < 9) // Verifica se a contagem é menor que 9
            
                contagem++; // Incrementa a contagem
            else
                contagem = 0; // Reinicia a contagem caso seja maior que 9
            
            printnum(led_r, led_g, led_b, contagem); // Ilustra a contagem nos LEDs
            printf("Contagem: %d\n", contagem); // Imprime a contagem no terminal
        }
        if (gpio == buttonB) { // Caso a interrupção seja no botão B
            if (contagem > 0) // Verifica se a contagem é maior que 0
                contagem--; // Decrementa a contagem
            else
                contagem = 0; // Mantém a contagem em 0 (não permite valores negativos)
            printnum(led_r, led_g, led_b, contagem); // Ilustra a contagem nos LEDs
            printf("Contagem: %d\n", contagem); // Imprime a contagem no terminal
        }
        last_interrupt_time = current_time;  // Atualiza o tempo da última interrupção
    }
}

static inline void put_pixel(uint32_t pixel_grb) // Função para atualizar um LED
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u); // Atualiza o LED com a cor fornecida
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) // Função para converter RGB para uint32_t
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b); // Retorna a cor em formato uint32_t 
}

void printnum(uint8_t r, uint8_t g, uint8_t b, int contagem) // Função para imprimir o número da contagem na matriz WS2812
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define os padrões de LEDs para os números de 0 a 9
    bool num_0[numLeds] = { 
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 1, 1, 0
    };

    bool num_1[numLeds] = { 
        0, 1, 1, 1, 0, 
        0, 0, 1, 0, 0, 
        0, 0, 1, 0, 0, 
        0, 1, 1, 0, 0, 
        0, 0, 1, 0, 0
    };

    bool num_2[numLeds] = {
        0, 1, 1, 1, 0, 
        0, 1, 0, 0, 0, 
        0, 1, 1, 1, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0
    };

    bool num_3[numLeds] = {
        0, 1, 1, 1, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0
    };

    bool num_4[numLeds] = {
        0, 1, 0, 0, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 0, 1, 0
    };

    bool num_5[numLeds] = {
        0, 1, 1, 1, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 1, 0, 0, 0, 
        0, 1, 1, 1, 0
    };

    bool num_6[numLeds] = {
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 1, 0, 0, 0, 
        0, 1, 1, 1, 0
    };

    bool num_7[numLeds] = {
        0, 1, 0, 0, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 1, 1, 0
    };

    bool num_8[numLeds] = {
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 1, 1, 0
    };

    bool num_9[numLeds] = {
        0, 1, 1, 1, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 1, 1, 0
    };

    // Array de ponteiros para os números de 0 a 9
    bool *numeros[10] = {
        num_0, num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9
    };

    bool *num = numeros[contagem];  // Seleciona o padrão de leds com base na contagem
    // Atualiza os LEDs com base no padrão do número escolhido
    for (int i = 0; i < numLeds; i++)
    {
        if (num[i]) 
            put_pixel(color);  // Liga o LED
        else
            put_pixel(0);  // Desliga o LED
    }
}
