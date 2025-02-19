// Bibliotecas
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// Definições de pinos
#define VRX_PIN 27  
#define VRY_PIN 26  
#define SW_PIN 22   
#define GREEN_LED 11 
#define BLUE_LED 12  
#define RED_LED 13   
#define BUTTON 5     

// Configurações do display
#define I2C_PORT i2c1
#define I2C_SDA 14 
#define I2C_SCL 15 
#define DISPLAY_ADDR 0x3C // Endereço do display OLED

// Configurações do joystick
#define CENTER_VALUE 2048 // Valor central do ADC
#define DEAD_ZONE 200      // Zona morta do joystick

volatile uint32_t last_time = 0; // Último tempo registrado para debounce
volatile bool leds_active = true; // Estado dos LEDs PWM
volatile bool display_border = false; // Estado da borda do display
ssd1306_t display; // Estrutura do display OLED

// Função de interrupção para os botões
void button_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_time > 250) { // Implementação de debounce
        last_time = current_time;
        if (gpio == SW_PIN) {
            gpio_put(GREEN_LED, !gpio_get(GREEN_LED)); // Alterna LED verde
            display_border = !display_border; // Alterna borda do display
        }
        if (gpio == BUTTON) {
            leds_active = !leds_active; // Alterna estado dos LEDs PWM
        }
    }
}

// Inicializa um pino como PWM
uint init_pwm(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, wrap);
    pwm_set_enabled(slice, true);
    return slice;
}

// Mapeia um valor de um intervalo para outro
int map_value(int value, int min_input, int max_input, int min_output, int max_output) {
    return (value - min_input) * (max_output - min_output) / (max_input - min_input) + min_output;
}

int main() {
    stdio_init_all();
    uint wrap = 4095; // Valor máximo do ADC (12 bits)

    // Inicializa os LEDs como PWM
    init_pwm(BLUE_LED, wrap);
    init_pwm(RED_LED, wrap);

    // Inicializa o ADC e os pinos do joystick
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    // Configuração dos botões
    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);
    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);
    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);

    // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(SW_PIN, GPIO_IRQ_EDGE_FALL, true, &button_handler);
    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_handler);

    // Inicializa o barramento I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa e configura o display OLED
    ssd1306_init(&display, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_send_data(&display);

    while (true) {
        // Lê os valores do joystick
        adc_select_input(0);
        uint16_t vry_value = adc_read();
        adc_select_input(1);
        uint16_t vrx_value = adc_read();

        // Configuração inicial do PWM
        uint blue_pwm = 0, red_pwm = 0;
        if (leds_active) {
            // Ajusta intensidade do LED azul
            if (vry_value < CENTER_VALUE - DEAD_ZONE)
                blue_pwm = map_value(vry_value, 0, CENTER_VALUE - DEAD_ZONE, wrap, 0);
            else if (vry_value > CENTER_VALUE + DEAD_ZONE)
                blue_pwm = map_value(vry_value, CENTER_VALUE + DEAD_ZONE, wrap, 0, wrap);

            // Ajusta intensidade do LED vermelho
            if (vrx_value < CENTER_VALUE - DEAD_ZONE)
                red_pwm = map_value(vrx_value, 0, CENTER_VALUE - DEAD_ZONE, wrap, 0);
            else if (vrx_value > CENTER_VALUE + DEAD_ZONE)
                red_pwm = map_value(vrx_value, CENTER_VALUE + DEAD_ZONE, wrap, 0, wrap);
        }

        // Mapeia os valores do joystick para a posição no display
        uint x = map_value(vrx_value, 0, 4095, 0, 119);
        uint y = map_value(vry_value, 0, 4095, 55, 0);

        // Ajusta limites do cursor
        x = (x == 118) ? 116 : (x == 0) ? 4 : x;
        y = (y == 55) ? 52 : (y == 1) ? 4 : y;

        ssd1306_fill(&display, false);

        // Configura a borda do display
        if (display_border) {
            ssd1306_rect(&display, 0, 0, 128, 64, true, false);
        }
        ssd1306_rect(&display, y, x, 8, 8, true, true);
        ssd1306_send_data(&display);

        // Ajusta os níveis de PWM
        pwm_set_gpio_level(BLUE_LED, blue_pwm);
        pwm_set_gpio_level(RED_LED, red_pwm);

        printf("VRX: %u, VRY: %u, Red PWM: %d, Blue PWM: %d, X: %d, Y: %d\n", vrx_value, vry_value, red_pwm, blue_pwm, x, y);
        sleep_ms(100);
    }
}
