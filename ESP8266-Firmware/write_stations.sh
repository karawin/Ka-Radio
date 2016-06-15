#!/bin/bash
sudo /opt/Espressif/esp-open-sdk/esptool/esptool.py --port /dev/ttyUSB0  write_flash 0x3F0000  database.bin
