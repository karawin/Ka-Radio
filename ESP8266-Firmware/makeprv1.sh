#!/bin/bash
#make COMPILE=gcc BOOT=new SPI_SIZE_MAP=4 APP=1
#cd webpage
#./generate.sh
#cd ..
make NAME=prv BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=4
