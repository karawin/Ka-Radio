#!/bin/bash
export SDK_PATH=/media/sf_jp/Documents/GitHub/ESP8266_RTOS_SDK-master
export BIN_PATH=./bin
make clean
./make1.sh
make clean
./make2.sh

