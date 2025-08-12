# 🛠 Introdução do Projeto

Este projeto consiste na criação de um sistema embarcado interativo focado em entretenimento, que oferece quatro minijogos distintos controlados por movimentos detectados via sensor MPU6050. A interface visual é apresentada em um display OLED SSD1306, e a navegação ocorre por meio de botões físicos. O sistema também possui armazenamento em cartão SD para garantir a persistência dos dados gerados durante o uso, promovendo uma experiência contínua e responsiva desde a seleção do jogo até sua execução.

# 🎯 Objetivos

O objetivo principal é desenvolver um sistema funcional que integre controles por sensor de movimento, interface gráfica e armazenamento persistente. Para isso, busca-se:
- Construir quatro minijogos que utilizem a inclinação para controle.
- Implementar um menu de fácil navegação, operado por dois botões físicos.
- Gerenciar as tarefas do sistema com FreeRTOS para garantir fluidez.
- Integrar os componentes eletrônicos via protocolos I2C e SPI.
- Adicionar feedback sonoro por meio de um buzzer PWM.
- Assegurar o registro das pontuações no cartão SD.

# 🚀 Diagramas

- [Diagrama de Blocos](https://github.com/GislanyDias/ProjetoEmbarcados/blob/main/diagramas/diagrama-blocos.pdf)
- [Diagrama Esquemático](https://github.com/GislanyDias/ProjetoEmbarcados/blob/main/diagramas/Schematic.pdf)


# 🕹️ Jogos Implementados

- Dodge the Blocks - Desvie de obstáculos movendo o dispositivo lateralmente.
- Tilt Maze - Percorra um labirinto inclinando o dispositivo.
- Snake Tilt - Versão do clássico jogo da cobra controlada por inclinação.
- Paddle Pong - Rebata a bola com uma raquete controlada por movimento.


# 📁 Bibliotecas desenvolvidas

- **button** - Sistema avançado de entrada digital
- **buzzer** - Gerador de efeitos sonoros
- **games** - Jogos desenvolvidos
- **i2clib** - Camada de abstração para comunicação I2C
- **mpu6050** - Biblioteca para aquisição de dados inerciais via I2C
- **ssd1306** - Controlador avançado para display OLED 128x64


# 🛠️ Componentes utilizados
| Componente       | Interface | Função                                |
|------------------|-----------|---------------------------------------|
| ESP32            | -         | MCU principal                         |
| MPU6050          | I2C       | Acelerômetro/giroscópio               |
| SSD1306 (OLED)   | I2C       | Display 128x64 pixels                 |
| Módulo SD Card   | SPI       | Armazenamento de recordes             |
| Buzzer           | GPIO/PWM  | Efeitos sonoros                       |
| 2 Botões         | GPIO      | Navegação no menu                     |


# 🛠️ Funcionamento de cada biblioteca

- `button`:
### 📌 Funcionalidades
- ✅ **Interrupções** para resposta instantânea  
- 🔄 **Debounce em software** (filtro de ruído)  
- 📨 **Fila FreeRTOS** (comunicação thread-safe)  
- ⚙️ **Multi-botão** com pull-up/down configurável  

### 🔄 Fluxo de Operação
1. `init_buttons_isr()` - Configura GPIO e interrupções  
2. **ISR** envia eventos para a fila ao pressionar  
3. `button_get_event()` - Task principal lê eventos  

### 💻 Uso Básico
```c
button_event_data_t event;
if (button_get_event(&event) == ESP_OK) {
    // Tratar evento do botão
}
