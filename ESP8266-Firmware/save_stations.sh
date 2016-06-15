#!/bin/bash
sudo /opt/Espressif/esp-open-sdk/esptool/esptool.py --port /dev/ttyUSB0  read_flash 0x3F0000 0xc000 database.bin
