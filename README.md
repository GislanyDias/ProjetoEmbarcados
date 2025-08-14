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

# 📸 Video Demonstração

- [Video no Youtube](https://youtube.com/shorts/o0RamJytcEc)

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

## `button`:
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
```
button_event_data_t event;
if (button_get_event(&event) == ESP_OK) {
    // Tratar evento do botão
}
```


## `buzzer`:
### 📌 Funcionalidades
- 🔊 Controle de buzzer via PWM usando LEDC (8 bits, 5 kHz)
- 🎵 Geração de tons com frequência e duração configuráveis
- 🎮 Sons pré-definidos para eventos (game over, menu, pontuação, level up)
- 🛡 Proteção para evitar frequências muito baixas (<150 Hz)
- 📝 Log de erros ao definir frequências inválidas

### 🔄 Fluxo de Operação
- buzzer_init() – Configura o timer e canal LEDC para o buzzer com duty inicial zero
- play_tone(freq, dur) – Define a frequência, aciona o buzzer e aguarda o tempo especificado
- Funções de evento (play_game_over(), play_menu_select(), etc.) chamam play_tone() com sequências específicas

### 💻 Uso Básico
```
Copiar
Editar
buzzer_init();
play_tone(1000, 500); // Toca 1 kHz por 500 ms
play_game_over();     // Executa som de game over
```


## `i2clib`:
### 📌 Funcionalidades
- ⚡ Inicialização do barramento I²C no modo mestre
- 🔍 Varredura de dispositivos I²C para detectar endereços ativos
- 🛠 Configuração com pull-up interno para SDA e SCL
- 📊 Estados do barramento definidos por i2c_state_t (ativo, dormindo, travado, erro, etc.)
- 🔄 Contador de recuperação (i2c_recovery_count) para monitoramento
  
### 🔄 Fluxo de Operação
- i2c_init() – Configura pinos SDA/SCL, velocidade e instala o driver I²C
- i2c_scan() – Percorre endereços de 0x01 a 0x7E enviando comando de escrita
- Para cada endereço com resposta positiva (ESP_OK), incrementa a contagem de dispositivos encontrados

### 💻 Uso Básico
```
if (i2c_init() == ESP_OK) {
    i2c_scan(); // Lista dispositivos conectados ao barramento
}
```




## `mpu6050`:
### 📌 Funcionalidades
- ⚡ Inicialização do barramento I²C para comunicação com o MPU6050
- ✍ Escrita em registradores do sensor (mpu6050_write_byte)
- 📥 Leitura de um ou múltiplos bytes de registradores (mpu6050_read_byte, mpu6050_read_bytes)
- 🔄 Configuração inicial do MPU6050 (mpu6050_init)
- 🔢 Conversão de dados brutos para valores em g, °/s e °C (mpu6050_convert_data)
  
### 🔄 Fluxo de Operação
- i2c_master_init() – Configura e inicia o driver I²C no modo mestre
- mpu6050_init() – Verifica ID do sensor, acorda do modo sleep e configura parâmetros
- mpu6050_read_all() – Lê 14 bytes com dados de acelerômetro, giroscópio e temperatura

### 💻 Uso Básico
```
i2c_master_init();
if (mpu6050_init() == ESP_OK) {
    float ax, ay, az;
    mpu6050_read_acceleration(&ax, &ay, &az);
}
```
