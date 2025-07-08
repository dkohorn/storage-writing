#include "sd_card.h"
#include "driver/i2c.h"

void app_main(void) {
    //I2C configuration
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,                
        .sda_io_num = 8,                        
        .scl_io_num = 9,                        
        .sda_pullup_en = GPIO_PULLUP_ENABLE,    
        .scl_pullup_en = GPIO_PULLUP_ENABLE,    
        .master.clk_speed = 400000,      
    };
    i2c_param_config(0, &i2c_conf);
    i2c_driver_install(0, i2c_conf.mode, 0, 0, 0);

    //Pull CS pin low using I2C
    uint8_t write_buf = 0x01;
    i2c_master_write_to_device(0, 0x24, &write_buf, 1, 1000 / portTICK_PERIOD_MS);
    write_buf = 0x0A;
    i2c_master_write_to_device(0, 0x38, &write_buf, 1, 1000 / portTICK_PERIOD_MS);

    //SPI bus init
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI, 
        .miso_io_num = PIN_NUM_MISO, 
        .sclk_io_num = PIN_NUM_CLK,  
        .quadwp_io_num = -1,         // Not used
        .quadhd_io_num = -1,         // Not used
        .max_transfer_sz = 4000,     
    };
    spi_bus_initialize(CONFIG_SPI_BUS, &bus_cfg, SDSPI_DEFAULT_DMA);

    //!Library function
    sd_init();
        
    //Write to and read from a file on the SD
    const char *fpath = MOUNT_POINT "/test.txt";
    FILE *f = fopen(fpath, "w");
    fprintf(f, "Luke, I am your father...");
    fclose(f);

    char line[64];
    f = fopen(fpath, "r");
    printf("Read from file: \n"); 
    fgets(line, sizeof(line), f);
    printf("%s\n", line);
    fclose(f);

    //!Library function
    sd_eject();
    
    spi_bus_free(CONFIG_SPI_BUS);
}

