#ifndef DRIVER_SDCARD_H
#define DRIVER_SDCARD_H

#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"

// Caminho do ponto de montagem do cartão SD
#define SDCARD_MOUNT_POINT      "/sdcard"

// Pinos SPI para o cartão SD
#define SDCARD_PIN_MISO         GPIO_NUM_2
#define SDCARD_PIN_MOSI         GPIO_NUM_15
#define SDCARD_PIN_SCLK         GPIO_NUM_14
#define SDCARD_PIN_CS           GPIO_NUM_13

// Inicializa o cartão SD e monta o sistema de arquivos
void sdcard_init(void);

// Escreve conteúdo em um arquivo no cartão SD
void sdcard_write_file(const char *file_path, const char *content);

// Lê conteúdo de um arquivo no cartão SD
void sdcard_read_file(const char *file_path, char *out_content, int max_size);

#endif // DRIVER_SDCARD_H
