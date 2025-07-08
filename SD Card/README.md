This project provides a minimal library to aid in the use of an SD card.

Running main.c will test the functionality of this library paired with a ESP32-S3-Touch-LCD-4.3-B board, for file manipulation.

This project has pin configurations defaulted for the board mentioned above, however they can be altered through the kconfig.projbuild:
MOSI = 11
MISO = 13
CLK = 12
CS = -1

The CS pin on this board is controlled externally via a CH422G chip via I2C. It must be set to -1 to ensure this 3rd party control works properly. 
This library will only work with SPI communication, ensure that the proper SPI bus is selected in the kconfig.projbuild. 
The card has a MOUNT_POINT defined within the library, ensure this is used when accessing the SD card.