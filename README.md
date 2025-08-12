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

- [Diagrama de Blocos](https://www.google.com)
- [Diagrama Esquemático](https://github.com/GislanyDias/ProjetoEmbarcados/blob/main/diagramas/Schematic.pdf)


# 🕹️ Jogos Implementados

- Dodge the Blocks - Desvie de obstáculos movendo o dispositivo lateralmente.
- Tilt Maze - Percorra um labirinto inclinando o dispositivo.
- Snake Tilt - Versão do clássico jogo da cobra controlada por inclinação.
- Paddle Pong - Rebata a bola com uma raquete controlada por movimento.
