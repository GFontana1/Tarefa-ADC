# Tarefa-ADC - Embarcatech

## Descrição do projeto

O programa permite ao usuário controlar um ponto em um display ao mesmo tempo que acende uma led por pwm conforme o ponto se move ao utilizar:

- 1 LED RGB
- 1 Display SSD1306
- 1 Joystick analógico
- 1 Push Button


## Funcionalidades

- Controle do LED RGB a partir do joystick

- Exibição de um ponto no display, o qual pode ser movido com uso do joystick

- Efeito de borda alternável no display

- Interrupções para controle do LED verde e ativação/desativação do PWM


## Como rodar o código

1. *Necessário para compilar o código:*
    - Ter uma placa Raspberry Pi Pico W que contenha os componentes descritos na *Descrição do projeto*.
    - Ter o SDK do Raspberry Pi Pico W configurado.
    - Compilar o código utilizando CMake e GCC ARM.

2. *Transferir para o Raspberry Pi Pico W*:
   - Conecte sua placa ao computador com ela no modo **Bootsel**.
   - Monte a unidade **RPI-RP2** no computador.
   - Copie o arquivo compilado `.uf2` para a unidade montada ou aperte em Run na interface do VSCode caso tenha configurado a placa com o Zadig.
  

## Vídeo de demonstração: