#include "ssd.h"
#include <string.h>

static const char *TAG = "SD_CARD_DRIVER";

// Handle para o cartão SD
static sdmmc_card_t *card;

// Handle para o host SPI
static sdmmc_host_t host = {
    .flags = SDMMC_HOST_FLAG_SPI,
    .slot = 1, // Usando slot 1 para SPI
};

// Configuração do barramento SPI
static spi_bus_config_t bus_config = {
    .mosi_io_num = SDCARD_PIN_MOSI,
    .miso_io_num = SDCARD_PIN_MISO,
    .sclk_io_num = SDCARD_PIN_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 4000,
};

// Configuração do slot SPI
static sdspi_device_config_t slot_config = {
    .host_id = host.slot,
    .gpio_cs = SDCARD_PIN_CS,
    .gpio_cd = -1,
    .gpio_wp = -1,
    .gpio_int = -1,
};

// Configuração de montagem do sistema de arquivos VFS
static esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = true,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024,
};

//----------------------------------------------------------------------------------------------------------------------
// Implementação das funções
//----------------------------------------------------------------------------------------------------------------------

void sdcard_init(void) {
    esp_err_t ret;

    // Inicializa o barramento SPI
    ret = spi_bus_initialize(host.slot, &bus_config, SPI_DMA_CHAN_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao inicializar o barramento SPI (%s).", esp_err_to_name(ret));
        return;
    }

    // Monta o sistema de arquivos FAT no cartão SD
    ret = esp_vfs_fat_sdspi_mount(SDCARD_MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Falha ao montar o sistema de arquivos. Você pode precisar formatar o cartão.");
        } else {
            ESP_LOGE(TAG, "Falha ao inicializar o cartão (%s). O cartão pode estar com o barramento SPI incorreto.", esp_err_to_name(ret));
        }
        return;
    }

    // Cartão montado com sucesso
    ESP_LOGI(TAG, "Cartão SD montado no caminho: %s", SDCARD_MOUNT_POINT);

    // Exibe informações do cartão
    sdmmc_card_print_info(stdout, card);
}

void sdcard_write_file(const char *file_path, const char *content) {
    ESP_LOGI(TAG, "Abrindo arquivo para escrita: %s", file_path);

    // Constrói o caminho completo do arquivo
    char full_path[128];
    snprintf(full_path, sizeof(full_path), "%s/%s", SDCARD_MOUNT_POINT, file_path);

    FILE *f = fopen(full_path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Falha ao abrir arquivo para escrita.");
        return;
    }

    fprintf(f, "%s", content);
    fclose(f);
    ESP_LOGI(TAG, "Arquivo escrito com sucesso.");
}

void sdcard_read_file(const char *file_path, char *out_content, int max_size) {
    ESP_LOGI(TAG, "Abrindo arquivo para leitura: %s", file_path);

    // Constrói o caminho completo do arquivo
    char full_path[128];
    snprintf(full_path, sizeof(full_path), "%s/%s", SDCARD_MOUNT_POINT, file_path);

    FILE *f = fopen(full_path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Falha ao abrir arquivo para leitura.");
        out_content[0] = '\0';
        return;
    }

    // Lê o conteúdo do arquivo
    int read_bytes = fread(out_content, 1, max_size - 1, f);
    out_content[read_bytes] = '\0';
    fclose(f);

    ESP_LOGI(TAG, "Arquivo lido com sucesso. Conteúdo: %s", out_content);
}