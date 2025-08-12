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

- **button** -
- **buzzer** -
- **games** -
- **i2clib** -
- **mpu6050** -
- **ssd1306** -

- ### MPU6050 Driver
`components/mpu6050`  
Biblioteca para aquisição de dados inerciais via I2C com:
- Calibração automática de offset
- Filtro complementar para fusão sensor (aceleração + giroscópio)
- Saída em unidades padrão (g, °/s)
- Taxa de amostragem configurável (até 1kHz)

### SSD1306 OLED Driver
`components/ssd1306`  
Controlador avançado para display OLED 128x64 com:
- Sistema de renderização por buffer duplo
- Fontes bitmap customizáveis (incluindo 5x7, 8x16)
- Primitivas gráficas (linhas, círculos, sprites)
- Otimização para atualização parcial da tela

### SD Card Manager
`components/sd_card`  
Sistema de armazenamento baseado em FatFS com:
- Interface SPI otimizada para ESP32
- Sistema de arquivos com journaling
- Gerenciamento de recordes em arquivos JSON
- Recuperação de falhas por checksum CRC32

### Buzzer PWM Controller
`components/buzzer`  
Gerador de efeitos sonoros com:
- Banco de melodias em formato RTTTL
- Controle de volume por duty cycle
- Efeitos especiais (vibrato, glissando)
- Sistema não-bloqueante baseado em callbacks

### Button Handler
`components/button`  
Sistema avançado de entrada digital com:
- Debounce digital (adaptativo)
- Detecção de eventos (press, release, hold)
- Configuração por polling ou interrupção
- Timeout configurável para long-press

### I2C Utilities
`components/i2c_commons`  
Camada de abstração para comunicação I2C com:
- Multiplexação de dispositivos no mesmo barramento
- Timeout dinâmico por dispositivo
- Logging de erros detalhado
- Wrappers para transações atômicas
