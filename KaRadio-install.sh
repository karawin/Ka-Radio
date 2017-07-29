#!/bin/bash
#How to generate KaRadio by yourself.

#Under windows 10, install Oracle vm VirtualBox and create a virtual machine with ubuntu (32bits) (3d acceleration, 48Mo video memory).
#Or a computer under Ubuntu.

echo Install https://github.com/pfalcon/esp-open-sdk:
#------------------------------------------------
cd
sudo apt-get  -y install make unrar-free autoconf automake libtool gcc g++ gperf \
    flex bison texinfo gawk ncurses-dev libexpat-dev python-dev python python-serial \
    sed git unzip bash help2man wget bzip2

sudo apt-get -y install libtool-bin

git clone --recursive https://github.com/pfalcon/esp-open-sdk.git

cd esp-open-sdk
make
#(be patient)
## read the end of the make to do the export
export PATH=/home/your home dir(to be adapted)/esp-open-sdk/xtensa-lx106-elf/bin:$PATH

echo install some tools
# https://github.com/espressif/esptool
# http://yui.github.io/yuicompressor/

sudo apt-get -y install python-pip
pip install esptool
sudo apt-get -y install yui-compressor

echo install Ka-Radio
#-----------------
cd
git clone --recursive https://github.com/karawin/Ka-Radio.git
cd Ka-Radio/ESP8266-Firmware
chmod +x *.sh
chmod +x webpage/*.sh

echo generate karadio
./genall.sh


