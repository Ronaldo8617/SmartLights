# Projeto de Controle de Iluminação Inteligente - Raspberry Pi Pico W - Parte 1

## 📌 Sumário  
- [📹 Demonstração](#-demonstração)  
- [🎯 Objetivo](#-objetivo)  
- [🛠️ Funcionalidades Obrigatórias](#️-funcionalidades-obrigatórias)  
- [✨ Funcionalidades Adicionais](#-funcionalidades-adicionais)  
- [📦 Componentes Utilizados](#-componentes-utilizados)  
- [⚙️ Compilação e Gravação](#️-compilação-e-gravação)  
- [📂 Estrutura do Código](#-estrutura-do-código)  
- [👨‍💻 Autor](#-autor)  

## 📹 Demonstração  
[clique aqui para acessar o vídeo](https://youtu.be/NvOw4scISNc)
 
Conteúdo do vídeo:  
- Apresentação do projeto  
- Explicação sobre o controle de iluminação via interface web  
- Demonstração ao vivo do controle dos quadrantes da matriz de LEDs  


## 🎯 Objetivo  
Desenvolver um sistema embarcado para controle remoto de iluminação residencial utilizando o Raspberry Pi Pico W e uma matriz de LEDs WS2818B. O sistema permite o controle individual de zonas de iluminação (quadrantes) através de uma interface web acessível por qualquer dispositivo conectado à rede Wi-Fi.  

## 🛠️ Funcionalidades Obrigatórias  
✅ Controle via interface web: Alterna os estados de 4 quadrantes da matriz de LEDs.  
✅ Conexão Wi-Fi: O Pico W conecta-se à rede e serve uma página web.  
✅ Atualização em tempo real: Mudanças nos quadrantes são refletidas instantaneamente na matriz.  
✅ Interface amigável: Botões para alternar cada quadrante diretamente na página HTML.  
✅ Estado visual dos quadrantes: LEDs amarelos indicam quadrante ativo; Desligados, inativo.  

## 📦 Componentes Utilizados  
- Microcontrolador: Raspberry Pi Pico W  
- Matriz de LEDs: WS2818B 5x5  
- Conectividade: Wi-Fi via CYW43  
- Ambiente de desenvolvimento: C/C++ com SDK do Pico e LWIP  

## ⚙️ Compilação e Gravação  
```bash
git clone https://github.com/Ronaldo8617/SmartLights
cd controle-iluminacao-inteligente
mkdir build
cd build
cmake ..
make
```

**Gravação:**  
Pelo ambiente do VScode compile e execute na placa de desnvovimento BitDogLab
Ou
Conecte o RP2040 no modo BOOTSEL e copie o `.uf2` gerado na pasta `build` para a unidade montada.

## 📂 Estrutura do Código  

```plaintext
SmartLights/  
├── lib/  
│   ├── font.h               # Fonte de caracteres do display  
│   ├── ssd1306.c, h         # Biblioteca do display OLED  
│   ├── buttons.c, h         # Configuração e debounce de botões  
│   ├── rgb.c, h             # Controle do LED RGB via PWM  
│   ├── display_init.c, h    # Inicialização e desenho no display  
│   ├── matrixws.c, h        # Controle da matriz WS2812B  
│   ├── ws2812b.pio.h        # Programa PIO da matriz WS2812B  
│   ├── buzzer.c, h          # Inicialização e controle do buzzer   
├── CMakeLists.txt           # Arquivo de configuração da build  
├── SmartLights.c            # Código principal (main)
├── Lwipopts.h               
├── README.md                # Documento de descrição do projeto  
```

## 👨‍💻 Autor  
**Nome:** Ronaldo César Santos Rocha  
**GitHub:** [Ronaldo8617]
