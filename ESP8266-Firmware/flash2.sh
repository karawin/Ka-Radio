#!/bin/bash
sudo /opt/Espressif/esp-open-sdk/esptool/esptool.py --baud 115200 --port /dev/ttyUSB0 write_flash  0x81000 "./bin/upgrade/user2.4096.new.4.bin" -fs 32m
