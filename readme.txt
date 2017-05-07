How to generate KaRadio by yourself.

Under windows 10, install Oracle vm VirtualBox and create a virtual machine with ubuntu (32bits) (3d acceleration, 48Mo video memory).
Or a computer under Ubuntu.

#Install https://github.com/pfalcon/esp-open-sdk:
#------------------------------------------------
sudo apt-get install make unrar-free autoconf automake libtool gcc g++ gperf \
    flex bison texinfo gawk ncurses-dev libexpat-dev python-dev python python-serial \
    sed git unzip bash help2man wget bzip2

sudo apt-get install libtool-bin

git clone --recursive https://github.com/pfalcon/esp-open-sdk.git

cd esp-open-sdk
make
#(be patient)
## read the end of the make to do the export
export PATH=/home/your home dir(to be adapted)/esp-open-sdk/xtensa-lx106-elf/bin:$PATH

#install some tools
# https://github.com/espressif/esptool
# http://yui.github.io/yuicompressor/

sudo apt-get install python-pip
pip install esptool
sudo apt-get install yui-compressor

#install Ka-Radio
#-----------------
cd
git clone --recursive https://github.com/karawin/Ka-Radio.git
cd Ka-Radio/ESP8266-Firmware
chmod +x *.sh
chmod +x webpage/*.sh

# generate karadio
./genall.sh


