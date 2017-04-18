# ** ESP8266 & VS1053 Wifi WebRadio ** #

### Ka-Radio, a WiFi shoutcast player based on ESP8266 and VS1053b chips
## Basic informations<BR/>
### Release 1.2 Built on 2017/04/13 <br/>
New features:<br/>
- new cli.info command<BR/>
- cli.list command updated.<BR/>
<BR/>
###Add On: add a lcd, a remote control, and other goodies: see https://github.com/karawin/Karadio-addons <br/>
<br/>
Attention:<br/>
The optional led is now on GPIO2. The blue led on the ESP8266 is blinking at the same rate.<BR/>
GPIO16 is now the Chip select for the external ram if any.<BR/>
The external ram is detected at boot time if present.<BR/><BR/>
To load this release, please flash user1.4096.new.4.bin at 0x1000 ,<BR/>
 user2.4096.new.4.bin at 0X81000 and blank.bin at	0x7e000 & 0x3fe000 <BR/>
After that, all next updates are done with the On The Air (OTA) feature.<BR/>
<BR/><BR/>
See also https://hackaday.io/project/11570-wifi-webradio-with-esp8266-and-vs1053 <br/>


#### Loading the esp8266
The binaries are on ESP8266-Firmware/bin/upgrade/

#### First use
- If the acces point of your router is not known, the webradio inits itself as an AP. Connect the wifi of your computer to the ssid "WifiWebRadio",  
- Browse to 192.168.4.1 to display the page, got to "setting" "Wifi" and configure your ssid ap, the password if any, the wanted IP or use dhcp if you know how to retrieve the dhcp given ip (terminal or scan of the network).
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



### History:
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
ADC   if control panel is not used, this input must be grounded.<br />
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
<br/>
Optional external ram (23LCV1024) support (1:CS/=GPIO16 2:MISO=GPIO12 3:NC 4:GND 5:MOSI=GPIO13 6:SCK=GPIO14 7:GND 8:3.3v)<br/>
If your chip has a /Vhold in place of /VBAT, the pin 7 must be wired to VCC (pin8)<br/>
Attention:<br>
	The optional led is now on GPIO2. The blue led on the ESP8266 is blinking at the same rate.<br/>
	GPIO16 is now the Chip select for the external ram if any.<br/>
	The external ram is detected at boot time if present.<br/>

## Used hardware
WiFi : ESP8266 (ESP-12 with 32Mbits flash)<br />
Additional MCU (as a bridge UART<=>UI): AVR<br />
Audio decoder: VS1053<br />

### update history:
Release 1.1.8 
- New uart command: cli.info Display nameSet, all icy and volume. Used to synchronize the lcd extension.
- cli.list command modified.

Release 1.1.7+ 
- Now Both AP SSID and AP Password are encoded to permit special characters like & : etc
- "Restore stations" corrected for some heavy lists
- Station information now gives the number of the current station
- retry client connection modified to avoid blocking situation.
- (1.1.7+) Volume offset on playlist was not working.

Release 1.1.6 
- Modification of the list of uart command. See uart and html interfaces
- New sys.patch command. Inhibit or permit (default) the load of a vs1053 patch for AAC stations.
- New sys.led command. Default is blinking mode, or Play mode: the led is on when a station is playing.
- Modification of the wifi.con command. Now the AP can be set in the AP mode (192.168.4.1) without the need of the web interface.
- New print button on the Stations panel: Print the list of stations, number and name.
- New logo ;-)

Release 1.1.5 
- Modification of the list of uart command. See uart and html interfaces
- New IR and LCD software
- Autoplay corrected

Release 1.1.4 
- Now 2 AP's can be set. The second one will be tested if the first is not detected.
- The Mac address of the radio is displayed
- An offset volume can be set for each station in Station editor.
- Pb on Autostart not correctly checked: corrected
- Added: a reset button for the equalizer
- Added a new uart command: cli.boot

Release 1.1.3 
- New html command: instant="http://your url"
- The path of a station can now include some & parameters.

Release 1.1.2 
- uart command list now can take a parameter: the number of the station to display.
.If no parameter, the complete list is sent.
- New wake and sleep features. Two modes: Time mode: "hh:mm" to wake or sleep at a given hour, or the delay mode "mm".
- Many minors html adjustments.
- Many bugs removed thanks to users feedback's.

Release 1.1.1 
- The user agent for http request can be set for some special streams.
   Example: http://pcradio.ru/player/listradio/pcradio_ru.xml which need a user agent= pcradio.
   This pcradio user agent is already implemented automatically in the code when a pcradio station is encountered.
- Better station start and stop. (no more strange noises).
- Many bugs removed thanks to users feedback's.

Release 1.1.0 
- Drag an drop on the stations list: rearrange the list by dragging a line to another one. Save the change when asked.
- Themes toggle between light blue and dark marron theme with a click on the logo.
- Sleep mode to stop the play after a delay.
- Functions on the web page are optimized. 
- Many minor improvements on the code and the web page.

Release 1.0.10 
- New uart interface command: cli.uart("x")
With x the uart baudrate at the next reset and permanently.
List of valid baudrate: 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 76880, 115200, 230400
The command will reply with
##CLI.UART= 115200# on the next reset.
- New Station selection with number
- Stations management in one page
- abort button on station edition.
Corrected:
- The I2S interface was not working. Thanks Discover1977 for the test

Release 1.0.9 
- New html interface see uart and html interfaces
- New I2S external DAC interface on the vs1053. The I2S is enabled by default to 48kHz. To change the speed permanently use the uart interface with the command cli.i2s("x") with x =0 for 48kHz, x=1 for 96kHz and x=2 for 192kHz
The i2s interface is available on the alientek vs1053 with i2s_sclk on SCLK (GPIO16), i2s_sdata on SDIN (GPIO07), i2s_mclk on MCLK (GPIO05), i2s_lrout on LRCK (GPIO04).
- New Autostart: If autoplay is checked on the web interface, the current station is started at power on even with no web browser.

Release 1.0.8 
- Corrected: 
Some chunked html stations was wrong.
If no metadata, the station name is displayed in place
A station not found is indicated in the name on the web interface

Release 1.0.7 
- Corrected: The webstation lists was wrong on empty station.

Release 1.0.6 
- New sdk patch from Espressif see http://bbs.espressif.com/viewtopic.php?f=46&t=2349 
- Now the maximum number of stations is 256
- Detection of external ram is working. If your chip has a /Vhold in place of /VBAT, the pin 7 must be wired to VCC (pin8)
- Save stations from... now working with the right filename for edge browser
- The AP Password field is masked.
- Increased spi clock for external ram
