#include "sd_card.h"

static const char *TAG = "SD Card";

sdmmc_card_t *card;
const char mount_point[] = MOUNT_POINT;
sdmmc_host_t host = SDSPI_HOST_DEFAULT();


/**
 * Can be used on initialization and to reset a file
 */
esp_err_t create_file(const char *path) {
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed create file");
        return ESP_FAIL;
    }
    fprintf(f, "Timestamp, Temperature, Pressure\n");

    fclose(f);
    return ESP_OK;
}

esp_err_t delete_file(const char *path) {
    // Check if destination file exists before renaming
    struct stat st;
    if (stat(path, &st) == 0){
        // Delete it if it exists
        unlink(path);
    }
    else {
        ESP_LOGE(TAG, "Failed to delete file");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t write_file(const char *path, float time, float temp, float pressure) {
    FILE *f = fopen(path, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }

    fprintf(f, "%f, %f, %f\n", time, temp, pressure);
    fclose(f);

    return ESP_OK;
}


/**
 * Export file data to connected USB
 */
esp_err_t export_file(const char *path) {
    FILE *f = fopen(path, "r"); 
    if (f == NULL) {
        ESP_LOGI(TAG, "Failed to open file for export");
    }

    char line[1024];

    while (fgets(line, sizeof(line), f)) {
        printf("%s", line); 
    }

    fclose(f);
    return ESP_OK;
}

/**
 * I2C master initialization
 */
esp_err_t i2c_master_init(void) {
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,                // Set to master mode
        .sda_io_num = I2C_MASTER_SDA_IO,        // Set SDA pin
        .scl_io_num = I2C_MASTER_SCL_IO,        // Set SCL pin
        .sda_pullup_en = GPIO_PULLUP_ENABLE,    // Enable SDA pull-up
        .scl_pullup_en = GPIO_PULLUP_ENABLE,    // Enable SCL pull-up
        .master.clk_speed = I2C_MASTER_FREQ_HZ, // Set I2C clock speed
    };

    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/**
 * Pull chip select pin low through the CH422G 
 */
static void pull_cs_low(void) {
    uint8_t write_buf = 0x01;
    i2c_master_write_to_device(I2C_MASTER_NUM, 0x24, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    write_buf = 0x0A;
    i2c_master_write_to_device(I2C_MASTER_NUM, 0x38, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}


/**
 * SPI bus initialization
 */
esp_err_t spi_bus_init(void) {
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI, // Set MOSI pin
        .miso_io_num = PIN_NUM_MISO, // Set MISO pin
        .sclk_io_num = PIN_NUM_CLK,  // Set SCLK pin
        .quadwp_io_num = -1,         // Not used
        .quadhd_io_num = -1,         // Not used
        .max_transfer_sz = 4000,     // Maximum transfer size
    };

    return spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
}

/**
 * Mount the SD card
 */
esp_err_t mount_sd(void) {
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true, // If mount fails, format the card
#else
        .format_if_mount_failed = false, // If mount fails, do not format card
#endif
        .max_files = 5,                   // Maximum number of files
        .allocation_unit_size = 16 * 1024 // Set allocation unit size
    };

    // Configure SD card slot
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS; // Set CS pin
    slot_config.host_id = host.slot;  // Set host ID

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            // Failed to mount filesystem
            ESP_LOGE(TAG, "Failed to mount filesystem. If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            // Failed to initialize the card
            ESP_LOGE(TAG, "Failed to initialize the card (%s). Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        ret = ESP_FAIL;
    }

    return ret;
}

esp_err_t eject_sd() {
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    spi_bus_free(host.slot);

    return ESP_OK;
}

esp_err_t sd_card_init() {
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGW(TAG, "I2C master initialized.");

    pull_cs_low();
    ESP_LOGW(TAG, "CS pulled low.");

    ESP_ERROR_CHECK(spi_bus_init());
    ESP_LOGW(TAG, "SPI bus initialized.");

    ESP_ERROR_CHECK(mount_sd());
    ESP_LOGW(TAG, "SD card mounted.");

    return ESP_OK;
}

esp_err_t sd_card_test()
{
    esp_err_t ret;
     const char *fpath = MOUNT_POINT "/datatest.csv";

    //Create a file
    ret = create_file(fpath);
    if (ret != ESP_OK) {
        return ESP_FAIL;
    }

    //Test writing twice
    ret = write_file(fpath, 105.4f, 69.1f, 14.7f);
    if (ret != ESP_OK) {
        return ESP_FAIL;
    }
    ret = write_file(fpath, 62.0f, 74.9f, 15.2f);
    if (ret != ESP_OK) {
        return ESP_FAIL;
    }

    //Export the file data
    ret = export_file(fpath);
    if (ret != ESP_OK) {
        return ESP_FAIL;
    }

    //Reset the file
    ret = create_file(fpath);
    if (ret != ESP_OK) {
        return ESP_FAIL;
    }

    //Ensure file is empty now
    ret = export_file(fpath);
    if (ret != ESP_OK) {
        return ESP_FAIL;
    }

    //Delete the file
    delete_file(fpath);
    if (ret != ESP_OK) {
        return ESP_FAIL;
    }

    //Check that file is properly deleted
    FILE *f = fopen(fpath, "r");
    if (f == NULL) {
        ESP_LOGI(TAG, "File properly deleted.");
    }
    else {
        ESP_LOGW(TAG, "File was NOT deleted.");
        fclose(f);
    }


    // All done, unmount partition and disable SPI peripheral
    eject_sd();
    ESP_LOGW(TAG, "SD Card ejected");
    
    return ESP_OK;
}