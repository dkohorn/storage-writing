#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/gpio.h"

static const char *TAG = "SDMMC";

void app_main(void)
{
    esp_err_t ret;
    sdmmc_card_t *card;
    const char mount_point[] = "/sdcard";

    ESP_LOGI(TAG, "Initializing SD card");

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // Use SDMMC peripheral
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // Optional: enable pull-ups (depends on your board)
    gpio_set_pull_mode(GPIO_NUM_14, GPIO_PULLUP_ONLY); // CLK
    gpio_set_pull_mode(GPIO_NUM_15, GPIO_PULLUP_ONLY); // CMD
    gpio_set_pull_mode(GPIO_NUM_2,  GPIO_PULLUP_ONLY); // D0
    gpio_set_pull_mode(GPIO_NUM_4,  GPIO_PULLUP_ONLY); // D1
    gpio_set_pull_mode(GPIO_NUM_12, GPIO_PULLUP_ONLY); // D2
    gpio_set_pull_mode(GPIO_NUM_13, GPIO_PULLUP_ONLY); // D3

    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "SD card mounted at %s", mount_point);

    // Write a file
    FILE *f = fopen("/sdcard/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello from ESP32-S3 with SDMMC!\n");
    fclose(f);
    ESP_LOGI(TAG, "File written");

    // Optionally unmount
    // esp_vfs_fat_sdcard_unmount(mount_point, card);
    // ESP_LOGI(TAG, "SD card unmounted");
}
