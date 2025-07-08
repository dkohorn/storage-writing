#include "usb_stick.h"

void app_main(void)
{
    // Create FreeRTOS primitives
    app_queue = xQueueCreate(5, sizeof(app_message_t));
    assert(app_queue);

    BaseType_t task_created = xTaskCreate(usb_task, "usb_task", 4096, NULL, 2, NULL);
    assert(task_created);

    // Init BOOT button: Pressing the button simulates app request to exit
    // It will disconnect the USB device and uninstall the MSC driver and USB Host Lib
    const gpio_config_t input_pin = {
        .pin_bit_mask = BIT64(APP_QUIT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&input_pin));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1));
    ESP_ERROR_CHECK(gpio_isr_handler_add(APP_QUIT_PIN, gpio_cb, NULL));

    ESP_LOGI(TAG, "Waiting for USB flash drive to be connected");
    msc_host_device_handle_t msc_device = NULL;
    msc_host_vfs_handle_t vfs_handle = NULL;

    // Perform all example operations in a loop to allow USB reconnections
    while (1) {
        app_message_t msg;
        xQueueReceive(app_queue, &msg, portMAX_DELAY);

        if (msg.id == APP_DEVICE_CONNECTED) {
            if (dev_present) {
                ESP_LOGW(TAG, "MSC Example handles only one device at a time");
            } else {
                // 0. Change flag
                dev_present = true;
                // 1. MSC flash drive connected. Open it and map it to Virtual File System
                ESP_ERROR_CHECK(msc_host_install_device(msg.data.new_dev_address, &msc_device));
                const esp_vfs_fat_mount_config_t mount_config = {
                    .format_if_mount_failed = false,
                    .max_files = 3,
                    .allocation_unit_size = 8192,
                };
                ESP_ERROR_CHECK(msc_host_vfs_register(msc_device, MOUNT_PATH, &mount_config, &vfs_handle));

                // 2. Print information about the connected disk
                msc_host_device_info_t info;
                ESP_ERROR_CHECK(msc_host_get_device_info(msc_device, &info));
                msc_host_print_descriptors(msc_device);
                print_device_info(&info);

                // 3. List all the files in root directory
                ESP_LOGI(TAG, "ls command output:");
                struct dirent *d;
                DIR *dh = opendir(MOUNT_PATH);
                assert(dh);
                while ((d = readdir(dh)) != NULL) {
                    printf("%s\n", d->d_name);
                }
                closedir(dh);

                // 4. The disk is mounted to Virtual File System, perform some basic demo file operation
                file_operations();

                // 5. Perform speed test
                speed_test();

                ESP_LOGI(TAG, "Example finished, you can disconnect the USB flash drive");
            }
        }
        if ((msg.id == APP_DEVICE_DISCONNECTED) || (msg.id == APP_QUIT)) {
            if (dev_present) {
                dev_present = false;
                if (vfs_handle) {
                    ESP_ERROR_CHECK(msc_host_vfs_unregister(vfs_handle));
                    vfs_handle = NULL;
                }
                if (msc_device) {
                    ESP_ERROR_CHECK(msc_host_uninstall_device(msc_device));
                    msc_device = NULL;
                }
            }
            if (msg.id == APP_QUIT) {
                // This will cause the usb_task to exit
                ESP_ERROR_CHECK(msc_host_uninstall());
                break;
            }
        }
    }

    ESP_LOGI(TAG, "Done");
    gpio_isr_handler_remove(APP_QUIT_PIN);
    vQueueDelete(app_queue);
}