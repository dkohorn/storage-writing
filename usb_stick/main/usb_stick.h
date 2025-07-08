/*
 ** Digital Rack Firmware. (c) 2024 Silver Falls Capital. All Rights Reserved
 **
 ** This code is unlicensed and may not be copied, used, modified,
 ** compiled, run, or distributed in any form without the explicit
 ** written permission of Silver Falls Capital.
 **
 ** Author(s): David Kohorn dkohorn@silverfallscapital.com
 **/
#ifndef _USB_STICK_
#define _USB_STICK_

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <dirent.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_log.h"
#include "usb_host.h"
#include "msc_host.h"
#include "msc_host_vfs.h"
#include "ffconf.h"
#include "errno.h"
#include "driver/gpio.h"

// Mount point for the USB
#define MOUNT_POINT "/usb"

/**
 * !Initialize SPI bus and mount the SD card
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t usb_stick_init();

/**
 * !Unmount the SD card and free the SPI bus
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t usb_stick_eject();

/**
 * !Format the SD card with FATFS
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t usb_stick_format();

/**
 * Create a new file or reset an existing file on the USB
 * @param path: File path on the USB
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t create_file(const char *path);

/**
 * Delete a file on the USB 
 * @param path: File path on the USB
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t delete_file(const char *path);

/**
 * Write to a file on the USB stick
 * @param path: File path on the USB
 * @param timestamp: The timestamp of a reading to write to the file
 * @param temp: The temperature of the reading to write to the file
 * @param pressure: The pressure of a reading to write to the file
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t write_file(const char *path, float timestamp, float temp, float pressure);

/**
 * Export data from an existing file on the USB
 * @param path: File path on the USB
 * @return: ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t export_file(const char *path);

#endif