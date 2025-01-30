# Interrupções com Raspberry Pi Pico W

Este repositório contém a implementação das atividades propostas na Unidade 4, Capítulo 4, sobre o uso de **interrupções** no **RP2040**, utilizando a placa **BitDogLab** e a linguagem **C**.

## Descrição
O projeto explora conceitos de interrupções, debounce de botões e controle de LEDs WS2812 e RGB, através das seguintes funcionalidades:

1. **Piscar do LED RGB**: O LED vermelho pisca **5 vezes por segundo**.
2. **Controle por Botões**:
   - O **Botão A** incrementa o número exibido na matriz de LEDs (0-9).
   - O **Botão B** decrementa o número exibido na matriz de LEDs (0-9).
3. **Matriz de LEDs WS2812**:
   - Exibe números de **0 a 9** em formato fixo ou criativo.

## Requisitos
Para rodar este projeto, são necessários:

- **Hardware**:
  - Placa **BitDogLab**
  - **Matriz 5x5 de LEDs WS2812** (GPIO 7)
  - **LED RGB** (GPIOs 11, 12 e 13)
  - **Botão A** (GPIO 5)
  - **Botão B** (GPIO 6)
- **Software**:
  - Visual Studio Code
  - Pico SDK
  - Biblioteca para controle de LEDs WS2812

## Estrutura do Projeto
```
Interrupcoes/
│── src/
│   ├── main.c   # Código principal
│   ├── irq.c    # Implementação das interrupções
│── include/
│── README.md
│── CMakeLists.txt   # Configuração do build com Pico SDK
```

## Como Executar
### 1. Configurar o Ambiente de Desenvolvimento
1. Instale o **Pico SDK** e configure o ambiente no **Visual Studio Code**.
2. Clone este repositório:
   ```sh
   git clone https://github.com/LucaScripts/Interrupoes.git
   cd Interrupcoes
   ```
3. Compile o projeto:
   ```sh
   mkdir build && cd build
   cmake ..
   make
   ```
4. Envie o binário para o Raspberry Pi Pico W.

## Funcionamento
### Controle dos LEDs e Botões
- O **LED vermelho** pisca continuamente **5 vezes por segundo**.
- O **Botão A** incrementa o número exibido na matriz de LEDs WS2812.
- O **Botão B** decrementa o número exibido na matriz de LEDs WS2812.
- A exibição dos números segue um padrão fixo ou um estilo criativo, desde que o número seja identificável.

### Implementação Técnica
- As funcionalidades dos **botões** são implementadas utilizando **interrupções (IRQ)**.
- O **debounce por software** evita leituras incorretas dos botões.
- O código está estruturado e bem comentado para facilitar a compreensão.

## Entrega
- **Código-Fonte**: Disponível neste repositório.
- **Vídeo de Demonstração**: Explicação do funcionamento e demonstração prática na placa BitDogLab.
  - Link para o vídeo: [(https://drive.google.com/file/d/1dmxRh9gSdziSYkaz_eXOMSuKhkNUOPur/view?usp=sharing)]

## Contribuição
Se desejar contribuir com melhorias ou sugestões, sinta-se à vontade para abrir um **pull request** ou relatar um **issue**.

## Autor
- **Lucas Dias da Silva**

