#include "sd_card.h"

static const char *TAG = "TEST";

esp_err_t sd_card_test()
{
    esp_err_t ret;
     const char *fpath = MOUNT_POINT "/datatest.csv";

    //Create a file
    ret = create_file(fpath);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create file");
        return ESP_FAIL;
    }

    //Test writing twice
    ret = write_file(fpath, 105.4f, 69.1f, 14.7f);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open file for writing(1)");
        return ESP_FAIL;
    }
    ret = write_file(fpath, 62.0f, 74.9f, 15.2f);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open file for writing(2)");
        return ESP_FAIL;
    }

    //Export the file data
    ret = export_file(fpath);
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "Failed to open file for export");
        return ESP_FAIL;
    }

    //Reset the file
    ret = create_file(fpath);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset file");
        return ESP_FAIL;
    }

    //Ensure file is empty now
    ret = export_file(fpath);
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "Failed to open file for export");
        return ESP_FAIL;
    }

    //Delete the file
    delete_file(fpath);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to delete file");
        return ESP_FAIL;
    }

    //Check that file is properly deleted
    FILE *f = fopen(fpath, "r");
    if (f != NULL) {
        ESP_LOGE(TAG, "File was NOT deleted.");
        fclose(f);
    }

    // All done, unmount partition and disable SPI peripheral
    sd_card_eject();
    ESP_LOGW(TAG, "SD Card ejected");
    
    return ESP_OK;
}

void app_main(void)
{
    // Initialize SD card 
    if(sd_card_init() == ESP_OK)
    {
        ESP_LOGW(TAG, "SD Card Initialized");
        // Test SD card functionality 
        sd_card_test();
    }

    
}

