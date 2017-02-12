#!/bin/bash
sudo /opt/Espressif/esp-open-sdk/esptool/esptool.py --baud 140000 --port /dev/ttyUSB0 write_flash 0x00000 "./bin/boot_v1.5.bin" 0x01000 "./bin/upgrade/user1.4096.new.4.bin" 0x81000 "./bin/upgrade/user2.4096.new.4.bin" -fs 32m
