#include "sd_card.h"

void app_main(void)
{
    // Initialize SD card 
    if(sd_card_init() == ESP_OK)
    {
        // Test SD card functionality 
        sd_card_test();
    }

    
}

