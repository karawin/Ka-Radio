#!/bin/bash
export SDK_PATH=/media/sf_jp/Documents/GitHub/ESP8266_RTOS_SDK-master
export BIN_PATH=./bin
make BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=4
