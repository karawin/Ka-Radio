# ** ESP8266 & VS1053 Wifi WebRadio ** #

### Ka-Radio, a WiFi shoutcast player based on ESP8266 and VS1053b chips</br>
See also the esp32 version at https://github.com/karawin/Ka-Radio32</br>

## Basic informations<BR/>
### Release 1.6.3- Built on 2017/12/27
New:<br/>
<ul>
	<li>1.6.3 R0: Correction for stations with the char & in path.</li>
	<li>1.6.2 R4: server NTP changed </li>
	<li>1.6.2: Add of the rssi (Received Signal Strength Indication -30:best, -99:worst) at top right of the web page. </li>
	<li>1.6.1: Click on the header to rewind to the top </li>
	<li>Header stays on top of the web page </li>
	<li>gzipped html tranfert. More stable and improved speed</li>
	<li>The page content follows the header size.</li>
</ul>
### Release 1.5- Built on 2017/08/07
New:<br/>
<ul>
	<li>Deeply tuned and optimized</li>
	<li>Added: a button erase on the station edit</li>
	<li>Little change of the menu labels.</li>
	<li>If the same meta is repeated in the stream, it is filtered</li>
	<li>telnet server now accepts the negociation.</li>
	<li>Only one task for all servers: websocket, web interface and telnet.</li>
	<li>Optimized threshold to start playing.It depends now of the size of the reception buffer.</li>
	<li>New web command: version, infos and list. See the interface.txt file on setting menu.</li>
    </ul>
    <p></br>Bugs resolved: (Thanks Kim)</p>
    <ul>
		<li>autoplay was broken</li><br>
		<li>monitor doesn't always start well.</li></br>
		<li>A bug in meta for low stream stations removed.</li><br>
    </ul>

### Release 1.4.2 - Built on 2017/08/01
New:<br/>
Unable to make the Espressif RTOS SDK 1.5.0-dev working, so I fed up.<br/>
- This 1.4.2 is able to play every 320Kb/s stations.<br/>
- New command: sys.version : Display the current Release and Revision.<br/>
- As always, more free ram memories. The websocket and telnet tasks are now one task for both.<br/>
- The web interface loading speed is improved.<br/>
- A telnet server multi clients on port 23. It is the same interface as the uart one. So the low level management can be done via the network and it offer a complete interface for remote addons.<br/>
- A deep modification of the memory management. More free room.<br/>
- new commands: help and sys.adc.<br/>
- The web volume command is now on a POST if the websocket is not ready.<br/>
- Play all 320Kb/s Stations<br/>
-new command: sys.version<br/>
<br/>
Bugs resolved:<br/>
- a problem on the web interface close resolved.<br/>
A lot!<br/>
1.4.1 crashed on stations reload and some other occasions.<br/>
Improved: the second AP connection.<br/>
Improved: low bitrate stations meta decoding.<br/>
cli.list modified. Now for a complete listing, each line begins with CLI.LISTNUM to avoid interference with addons.<br/>
<br/>
### Release 1.3.4 - Built on 2017/07/11
Bugs removed:<br/>
- Internal optimization on websocket. More free ram for the web interface multi-user.
- 1.3.3 A problem when a control panel and an addon are present is removed<BR/>
- 1.3.3 A problem with the AP2 password not working is removed (regression).<BR/>
<br/>
### Release 1.3.2 - Built on 2017/06/19
New features:<br/>
- The test task is replaced with an os timer to have more free memories
- Edit station now in modal window<br
- Added a mouse event on full url<br/>
- Many internal optimizations<br/>
- New control buttons on the web interface<br/>
- A potential huge bug removed<br/>
<br/>

###Add On: add a lcd, a remote control, and other goodies: see https://github.com/karawin/Karadio-addons <br/>
<br/>
Attention:<br/>
The optional led is now on GPIO2. The blue led on the ESP8266 is blinking at the same rate.<BR/>
GPIO16 is now the Chip select for the external ram if any.<BR/>
The external ram is detected at boot time if present.<BR/><BR/>
To load this release, please flash <BR/>
boot_v1.6.bin at 0x0000,<BR/>
user1.4096.new.4.bin at 0x1000 ,<BR/>
user2.4096.new.4.bin at 0X81000,<BR/>
esp_init_data_default.bin at 0x3FC000 <BR/>
and blank.bin at	0x3fe000 <BR/>
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
- Karadio can be controlled by the web interface or by the uart interface or by telnet. List of commands: type help
- See the list of command at http://karadio.karawin.fr/interface.txt

#### Feedback
Please tell me if you succeded or something more can be done, thanks.<br/>
The second step will add some hardware buttons (vol + -, station + -, play ...)<br/><br/>

<img src="https://github.com/karawin/ESP8266-WebRadio/blob/master/Images/webradio1mini.jpg" alt="screenshot" border=0> 
<img src="https://github.com/karawin/ESP8266-WebRadio/blob/master/Images/webradio2mini.jpg" alt="screenshot" border=0> 
<img src="https://github.com/karawin/ESP8266-WebRadio/blob/master/Images/webradio3mini.jpg" alt="screenshot" border=0> 

### Generate Ka-Radio
see the http://karadio.karawin.fr/readme.txt file.

### History:
- Based on https://github.com/PiotrSperka/ESP8266-WebRadio<br />
- New development based on the new https://github.com/espressif/ESP8266_RTOS_SDK<br />
- Compiled with [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk)<br />
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


