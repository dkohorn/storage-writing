#include "sd_card.h"

sdmmc_card_t *card;
const char mount_point[] = MOUNT_POINT;

esp_err_t sd_init(void) {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = CONFIG_SPI_BUS;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,     //If mount fails, format the card
        .max_files = 5,                     //Maximum number of files that can be opened
        .allocation_unit_size = 16 * 1024   //Allocation size for formatting the card if the mount fails
    };

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;  

    return esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
}

esp_err_t sd_eject() {
    return esp_vfs_fat_sdcard_unmount(mount_point, card);
}
