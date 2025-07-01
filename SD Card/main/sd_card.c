#include "sd_card.h"

sdmmc_card_t *card;
const char mount_point[] = MOUNT_POINT;
sdmmc_host_t host = SDSPI_HOST_DEFAULT();

//Create a new file or reset an existing file on the SD card
esp_err_t create_file(const char *path) {
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        return ESP_FAIL;
    }

    //Add header to the file
#ifdef CONFIG_FILE_TYPE_CSV
        fprintf(f, "Timestamp, Temperature, Pressure\n");
#endif
    fclose(f);
    return ESP_OK;
}

//Delete a file from the SD card
esp_err_t delete_file(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) { //Ensure file exists
        return ESP_FAIL;
    }
    unlink(path);
    return ESP_OK;
}

//Write a new reading entry to an existing file on the SD card
esp_err_t write_file(const char *path, float timestamp, float temp, float pressure) {
    FILE *f = fopen(path, "a");
    if (f == NULL) {
        return ESP_FAIL;
    }

    //Write data to the file
#ifdef CONFIG_FILE_TYPE_CSV
        fprintf(f, "%f, %f, %f\n", timestamp, temp, pressure);
#endif
    
    fclose(f);
    return ESP_OK;
}

//Export data from an existing file on the SD card
esp_err_t export_file(const char *path) {
    FILE *f = fopen(path, "r"); 
    if (f == NULL) {
        return ESP_FAIL;
    }

    //!Print data on a file as placeholder until further functionality is implemented
    printf("\n"); 
    char line[64];
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line); 
    }
    fclose(f);
    printf("\n"); 

    return ESP_OK;
}

//I2C master initialization for communicating with the CH422G chip on a Waveshare board
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

//Pull the CS pin low through the I2C connection
static void pull_cs_low_i2c(void) {
    uint8_t write_buf = 0x01;
    i2c_master_write_to_device(I2C_MASTER_NUM, 0x24, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    write_buf = 0x0A;
    i2c_master_write_to_device(I2C_MASTER_NUM, 0x38, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}


//Initialize the SPI bus to communicate with the SD card
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

//Mount the SD card
esp_err_t mount_sd(void) {
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,   // If mount fails, format the card
        .max_files = 5,                   
        .allocation_unit_size = 16 * 1024 
    };

    // Configure SD card slot
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS; 
    slot_config.host_id = host.slot;  

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    return ret;
}

esp_err_t sd_card_eject() {
    esp_err_t ret;

    ret = esp_vfs_fat_sdcard_unmount(mount_point, card);
    if (ret != ESP_OK) {           
        return ESP_FAIL;
    }
    ret = spi_bus_free(host.slot);
    if (ret != ESP_OK) {          
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t sd_card_init() {
    //These boards have the CS pin on their CH422G chip which needs to be controlled via I2C
#ifdef CONFIG_BOARD_TYPE_WAVESHARE_S3_TOUCH_LCD
        ESP_ERROR_CHECK(i2c_master_init());
        pull_cs_low_i2c();
#endif 
    
    ESP_ERROR_CHECK(spi_bus_init());
    ESP_ERROR_CHECK(mount_sd());

    return ESP_OK;
}