#!/bin/bash
sudo /opt/Espressif/esp-open-sdk/esptool/esptool.py --baud 140000 --port /dev/ttyUSB0 write_flash  0x03fe000 "./bin/blank.bin" 0x0fe000 "./bin/blank.bin" 0x7E000 "./bin/blank.bin" 0x3fc000 ./bin/esp_init_data_default.bin 0xFC000 ./bin/esp_init_data_default.bin -fs 32m
