#!/bin/bash
#make COMPILE=gcc BOOT=new SPI_SIZE_MAP=6 APP=1
export SDK_PATH=/media/sf_jp/Documents/GitHub/ESP8266_RTOS_SDK-master
export BIN_PATH=./bin
make BOOT=new APP=2 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=4
