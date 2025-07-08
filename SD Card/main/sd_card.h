#ifndef _SD_CARD_
#define _SD_CARD_

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#define MOUNT_POINT "/sdcard"

//Pin assignments for SD SPI interface
#define PIN_NUM_MOSI CONFIG_PIN_MOSI
#define PIN_NUM_MISO CONFIG_PIN_MISO 
#define PIN_NUM_CLK CONFIG_PIN_CLK  
#define PIN_NUM_CS CONFIG_PIN_CS     

/**
 * Mount the SD card
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t sd_init();

/**
 * Unmount the SD card
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t sd_eject();


#endif