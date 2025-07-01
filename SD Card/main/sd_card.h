#ifndef _SD_CARD_
#define _SD_CARD_

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/i2c.h"

// I2C configuration
#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0                        /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000               /*!< I2C master clock frequency      */
#define I2C_MASTER_TX_BUF_DISABLE 0             /*!< I2C master doesn't need buffer  */
#define I2C_MASTER_RX_BUF_DISABLE 0             /*!< I2C master doesn't need buffer  */
#define I2C_MASTER_TIMEOUT_MS 1000              /*!< I2C master timeout in milliseconds */

// Mount point for the SD card
#define MOUNT_POINT "/sdcard"

// Pin assignments for SD SPI interface
#define PIN_NUM_MISO CONFIG_EXAMPLE_PIN_MISO /*!< Pin number for MISO    */
#define PIN_NUM_MOSI CONFIG_EXAMPLE_PIN_MOSI /*!< Pin number for MOSI   */
#define PIN_NUM_CLK CONFIG_EXAMPLE_PIN_CLK   /*!< Pin number for CLK    */
#define PIN_NUM_CS CONFIG_EXAMPLE_PIN_CS     /*!< Pin number for CS  */


/**
 * Initialize SPI bus and mount the SD card
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t sd_card_init();

/**
 * Unmount the SD card and free the SPI bus
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t sd_card_eject();

/**
 * Create a new file or reset an existing file on the SD card
 * @param path: File path on the SD
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t create_file(const char *path);

/**
 * Delete a file on the SD card
 * @param path: File path on the SD
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t delete_file(const char *path);

/**
 * Write a new reading entry to an existing file on the SD card
 * @param path: File path on the SD
 * @param timestamp: The timestamp of a reading to write to the file
 * @param temp: The temperature of the reading to write to the file
 * @param pressure: The pressure of a reading to write to the file
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t write_file(const char *path, float timestamp, float temp, float pressure);

/**
 * Export data from an existing file on the SD card to a USB
 * @param path: File path on the SD
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t export_file(const char *path);


#endif
