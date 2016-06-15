#!/bin/bash
sudo /opt/Espressif/esp-open-sdk/esptool/esptool.py --baud 140000 --port /dev/ttyUSB0 write_flash  0x01000 "./bin/upgrade/user1.4096.new.6.bin" -fs 32m
