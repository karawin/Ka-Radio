# ** ESP8266 & VS1053 Wifi WebRadio** #
###Ka-Radio, a WiFi shoutcast player based on ESP8266 and VS1053b chips
##Basic informations
####Version: 1.0.4<br/>
Added:<br/>
Next and previous buttons,<br/>
Autoplay check box<br/>
"Now playing" removed, the title is now on the top of the page,<br/>
A hardware panel can be built. See  ESP8266-WebRadio/Hardware/KiCAD/controles/controles.pdf ,<br/>
Optimized software.<br/>
<br/>
This release is the end of the project.<br/>
In the future, a remote control will be added (433Mhz RF) based on arduino pro mini.<br/>
####Version: 1.0.3<br />
Added:<br/>
New html look,<br/>
Volume slider is now logarithmic,<br/>
Monitoring of the current station in the browser, usefull for mobile<br/>
Optimised ram usage.<br/>
Read chunked html stream, some stations use this kind of GET<br/>
First start updated,<br/>
It seems that the 320KB/s is still not perfect due to too few ram on the processor. Not so bad but very sensitive, 256kB/s is perfect.<br/>
####Version: 1.0.2<br />
Added: "Now playing" informations are now given with the help of a websocket. The display is immediatly updated on the browser.<br/>
Reading 320Kb/s is improved<br/>
Stability tested during many days and nights....<br/>
Video on https://youtu.be/cT8cGp9pvcs<br/>
Hackaday.io page: https://hackaday.io/project/11570-wifi-webradio-with-esp8266-and-vs1053 <br/>
Github page: https://github.com/karawin/ESP8266-WebRadio <br/>
#####Version: 1.0.1<br />
A static ip at the first start is a problem if you are not in 192.168.1.xxx network.<br />
The first start will now be in dhcp mode. See the allocated ip in the uart traces or scan your network.<br />
This can be done with fing for mobile for example: <br />
IOS: https://itunes.apple.com/fr/app/fing-network-scanner/id430921107?mt=8<br />
Android: https://play.google.com/store/apps/details?id=com.overlook.android.fing&hl=fr<br />

#####Version: 1.0.0 (KaraWin)<br />
Status: under design and development
### 2016, May (KaraWin)
First step, Final release done.
- Read up to 320 kB/s stream,
- New web page, Fit well on mobile or computer.
- Embedded wifi configuration via the web site (see below)
- Metadata displayed,
- all settings saved (sound setting, stations, wifi)
- server multi clients with automatic refresh for all,
- The stations can be dowloaded and uploaded within the web site,
- very quick responsive web by caching local information in the browser.
- additional hardware: a led indicates the state of the equipment (gpo16->resistor->led->3.3volt). Speed blink: init, 90%in 10%out: connexion in progress, 10% in 90%out: Equipment ready.
- ...

#### Loading the esp8266
- https://github.com/karawin/ESP8266-WebRadio/blob/master/ESP8266-Firmware/bin/boot_v1.5.bin at 0x0000
- https://github.com/karawin/ESP8266-WebRadio/blob/master/ESP8266-Firmware/bin/upgrade/user1.4096.new.6.bin at 0x1000

#### First use
- If the acces point of your router is not known, the webradio inits itself as an AP. Connect your wifi to the ssid "WifiWebRadio",  
- Browse to 192.164.4.1 to display the page, got to "setting" "Wifi" and configure your ssid ap, the password if any, the wanted IP or use dhcp if you know how to retrieve the dhcp given ip (terminal or scan of the network).
- In the gateway field, enter the ip address of your router.
- Validate. The equipment restart to the new configuration. Connect your wifi to your AP and browse to the ip given in configuration.
- Congratulation, you can edit your own station list. Dont forget to save your stations list in case of problem or for new equipments.
- if the AP is already know by the esp8266, the default ip is given by dhcp.
- a sample of stations list is on https://github.com/karawin/ESP8266-WebRadio/blob/master/ESP8266-Firmware/WebStations.txt . Can be uploaded via the web page.        

#### Feedback
Please tell me if you succeded or something more can be done, thanks.<br/>
The second step will add some hardware buttons (vol + -, station + -, play ...)<br/><br/>

<img src="https://github.com/karawin/ESP8266-WebRadio/blob/master/Images/webradio1mini.jpg" alt="screenshot" border=0> 
<img src="https://github.com/karawin/ESP8266-WebRadio/blob/master/Images/webradio2mini.jpg" alt="screenshot" border=0> 
<img src="https://github.com/karawin/ESP8266-WebRadio/blob/master/Images/webradio3mini.jpg" alt="screenshot" border=0> 



### 2016,April (Karawin):
- Based on https://github.com/PiotrSperka/ESP8266-WebRadio<br />
- New development based on the new https://github.com/espressif/ESP8266_RTOS_SDK<br />
- Compiled with [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk)<br />
- software improved, new web control<br />
- Compatible with mobile<br />
- Stable<br />
- tools to save and restore the stations database<br /><br />
- prototype made with:<br />
-- http://fr.aliexpress.com/item/MB102-Breadboard-Power-Supply-Module-3-3V-5V-For-Solderless-Breadboard/2027279953.html (0.74 euros)<br />
-- http://www.ebay.fr/itm/121775761053?_trksid=p2057872.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT  (3,09 euros)<br />
-- http://www.ebay.fr/itm/401046111343?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT  (6 euros)<br />
-- http://www.ebay.fr/itm/131683794035?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT  (1.05 euros)<br />
-- http://fr.aliexpress.com/item/USB-to-TTL-converter-UART-module-CH340G-CH340-3-3V-5V-switch/32392228218.html?detailNewVersion=2 (0.59 euro)<br />
-- some wires ...<br /><br />
- Wiring: <br />
From ESP8266_ESP12( 3.3 v) to VS1053 (5 v)<br />
REST<br />
ADC<br />
CH_PD to 3.3v<br />
GPIO16 (a 1Hz output)<br />
GPIO14 to VS1053 SCK<br />
GPIO12 to VS1053 MISO<br />
GPIO13 to VS1053 MOSI<br />
<br />
TXD to   CH340G UART rx<br />
RXD from CH340G UART tx<br />
GPIO05 to VS1053 XDCS<br />
GPIO04 to VS1053 DREQ<br />
GPIO00 to VS1053 XRST<br />
GPIO02<br />
GPIO15 to VS1053 XCS<br />
<br />
##Used hardware
WiFi and main MCU: ESP8266 (ESP-12 with 32Mbits flash)<br />
Additional MCU (as a bridge UART<=>UI): AVR<br />
Audio decoder: VS1053<br />
##TODO list
###ESP8266
-**[NORMAL]**Metadata processing<br />
-**[LOW]**External SRAM implementation<br />
-**[LOW]**Code cleanup and optimization
###AVR
-Hardware design<br />
-Code
##Changelog
###v0.13
-**[ESP8266]**Some kind of beta version of web interface is ready.<br />
-**[ESP8266]**Works saving and loading stations and settings to/from flash<br />
-**[ESP8266]**Added "How to run" document
###v0.12
-**[ESP8266]**Development of functions for saving and reading settings from flash "eeprom"<br />
-**[ESP8266]**Functions for driving GPIO16<br />
-**[ESP8266]**Development of webinterface<br />
-**[ESP8266]**Webclient is using socket
###v0.11
-**[Hardware]**Finished first revision of schematic (main board) and PCB<br />
-**[Hardware]**Added GERBER files for main board (NOT TESTED YET)<br />
-**[Hardware]**AVR is going to be only an interface between UART in ESP8266 and UI (which is LCD, encoder and buttons)
###v0.10
-**[ESP8266]**Support for user data (stations, config, etc.) storage in last 64kb of flash
-**[Hardware]**Schematic of main board
###v0.9
-**[ESP8266]**Sound settings through web interface<br />
-**[ESP8266]**Web interface improvements
###v0.8
-**[ESP8266]**Webserver uses sockets - improved stability<br />
-**[ESP8266]**Improvments of webinterface (now supports Instant Play)<br />
-**[ESP8266]**Improvments of webserver (simple POST parser)<br />
-**[ESP8266]**DNS lookup
###v0.7
-**[ESP8266]**Simple and rough sketch of VS1053 driver (working)<br />
-**[ESP8266]**Rough sketch of webclient is working<br />
-I can say that alpha version of player works for now, because I can listen to some SHOUTcast streams.
###v0.6
-**[ESP8266]**Started making new firmware based on esp_iot_rtos_sdk with bare LwIP and freeRTOS<br />
-**[Hardware]**New conception of hardware, where main MCU is ESP8266 and some small AVR is only to provide user interface (LCD, etc.) and configure ESP via UART<br />
-First tests of new firmware are very promising, because HTTP server based on Netconn is very responsive (much better than previous version of firmware)
###v0.5
-**[ESP8266]**Improved: HTTP server configuration<br />
-**[ESP8266]**Added: HTTP client UART output<br />
-**[STM32]**Added: Unclean sketch of firmware for basic testing purposes
###v0.4
-**[ESP8266]**Added: Simple CGI parser<br />
-**[ESP8266]**Added: ICY header parser
###v0.3
-**[ESP8266]**Added: Simple admin panel mockup<br />
-**[ESP8266]**Added: Connecting to client
###v0.2
-**[ESP8266]**Added: Running simple http server on port 80<br />
-**[ESP8266]**Added: Simple UART command parser for wifi configuration
###v0.1
-**[ESP8266]**Initial release - ESP8266 firmware based on cleaned nodemcu source (deleted lua, flash-fs, etc.)
