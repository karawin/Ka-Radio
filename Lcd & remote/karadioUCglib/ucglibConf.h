// ucglibConf.h
/*

>>> Before compiling: Please remove comment from the constructor of the
>>> connected graphics display (see below).

Use Universal 8bit Graphics Library, https://github.com/olikraus/ucglib/

*/
#include <SPI.h>
#include <Ucglib.h>

/*
  Hardware SPI Pins:
    Arduino Uno		sclk=13, data=11
    Arduino Due		sclk=76, data=75
    Arduino Mega	sclk=52, data=51
	Arduino Mini	sclk=13, data=11 (mosi)
	Arduino Micro	sclk=15, data=16
    
  >>> Please uncomment (and update) one of the following constructors. <<<  
*/
//Ucglib8BitPortD ucg(ucg_dev_ili9325_18x240x320_itdb02, ucg_ext_ili9325_18, /* wr= */ 18 , /* cd= */ 19 , /* cs= */ 17, /* reset= */ 16 );
//Ucglib8Bit ucg(ucg_dev_ili9325_18x240x320_itdb02, ucg_ext_ili9325_18, 0, 1, 2, 3, 4, 5, 6, 7, /* wr= */ 18 , /* cd= */ 19 , /* cs= */ 17, /* reset= */ 16 );

//Ucglib4WireSWSPI ucg(ucg_dev_ili9325_18x240x320_itdb02, ucg_ext_ili9325_18, /*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);	// not working
//Ucglib4WireSWSPI ucg(ucg_dev_ili9325_spi_18x240x320, ucg_ext_ili9325_spi_18, /*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);	// not working
//Ucglib3WireILI9325SWSPI ucg(ucg_dev_ili9325_spi_18x240x320, ucg_ext_ili9325_spi_18, /*sclk=*/ 13, /*data=*/ 11, /*cs=*/ 10, /*reset=*/ 8);	// not working
//Ucglib3WireILI9325SWSPI ucg(ucg_dev_ili9325_18x240x320_itdb02, ucg_ext_ili9325_18, /*sclk=*/ 13, /*data=*/ 11, /*cs=*/ 10, /*reset=*/ 8);	// not working

//Ucglib_ST7735_18x128x160_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
Ucglib_ST7735_18x128x160_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);

//Ucglib_ILI9163_18x128x128_SWSPI ucg(/*sclk=*/ 7, /*data=*/ 6, /*cd=*/ 5 , /*cs=*/ 3, /*reset=*/ 4);
//Ucglib_ILI9163_18x128x128_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);	/* HW SPI Adapter */

//Ucglib_ILI9341_18x240x320_SWSPI ucg(/*sclk=*/ 7, /*data=*/ 6, /*cd=*/ 5 , /*cs=*/ 3, /*reset=*/ 4);
//Ucglib_ILI9341_18x240x320_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_ILI9341_18x240x320_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_ILI9341_18x240x320_SWSPI ucg(/*sclk=*/ 4, /*data=*/ 3, /*cd=*/ 6 , /*cs=*/ 7, /*reset=*/ 5);	/* Elec Freaks Shield */

//Ucglib_SSD1351_18x128x128_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_SSD1351_18x128x128_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_SSD1351_18x128x128_FT_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_SSD1351_18x128x128_FT_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);

//Ucglib_PCF8833_16x132x132_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cs=*/ 9, /*reset=*/ 8);	/* linksprite board */
//Ucglib_PCF8833_16x132x132_HWSPI ucg(/*cs=*/ 9, /*reset=*/ 8);	/* linksprite board */

//Ucglib_LD50T6160_18x160x128_6Bit ucg( /*d0 =*/ d0, /*d1 =*/ d1, /*d2 =*/ d2, /*d3 =*/ d3, /*d4 =*/ d4, /*d5 =*/ d5, /*wr=*/ wr, /*cd=*/ cd, /*cs=*/ cs, /*reset=*/ reset);
//Ucglib_LD50T6160_18x160x128_6Bit ucg( /*d0 =*/ 16, /*d1 =*/ 17, /*d2 =*/ 18, /*d3 =*/ 19, /*d4 =*/ 20, /*d5 =*/ 21, /*wr=*/ 14, /*cd=*/ 15); /* Samsung 160x128 OLED with 6Bit minimal interface with Due */ 
//Ucglib_LD50T6160_18x160x128_6Bit ucg( /*d0 =*/ 5, /*d1 =*/ 4, /*d2 =*/ 3, /*d3 =*/ 2, /*d4 =*/ 1, /*d5 =*/ 0, /*wr=*/ 7, /*cd=*/ 6); /* Samsung 160x128 OLED with 6Bit minimal interface with Uno */ 

//Ucglib_SSD1331_18x96x64_UNIVISION_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_SSD1331_18x96x64_UNIVISION_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);

//Ucglib_SEPS225_16x128x128_UNIVISION_SWSPI ucg(/*sclk=*/ 13, /*data=*/ 11, /*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);
//Ucglib_SEPS225_16x128x128_UNIVISION_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Version 1.0: Initial release 01/2017 jp Cocatrix
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**** Instruction for 1.8" Serial 128X160 SPI TFT LCD Module Display + PCB Adapter Power IC SD Socket *************************
     ----------------------------------------------------------------------------------------------
NOTES:
Confirmed vendor:
lcd: https://www.aliexpress.com/item/J34-A96-Free-Shipping-1-8-Serial-128X160-SPI-TFT-LCD-Module-Display-PCB-Adapter-Power/32599685873.html?spm=2114.13010608.0.0.MvX4RI
cpu mini: http://www.ebay.fr/itm/Pro-Mini-Atmega328-5V-16M-Micro-controller-Board-for-Arduino-Compatible-Nano-new-/331809989705?hash=item4d416aa449
or cpu micro: https://www.aliexpress.com/item/Free-shipping-Atmega32u4-Game-Board-Module-Esplora-With-1PCS-Mini-USB-Cable-For-Arduino/1847119261.html?spm=2114.13010608.0.0.MvX4RI

MiniPRO 5v 16MHz:
 Pins:
#__      Function_________________________________
RESET    Reset (not used))
Rx  0    to the tx of the nodeMcu
Gnd      ground
Raw: 5 to 16 VDC Max

Micro pro 5v 16Mhz:
 Pins:
#__      Function_________________________________
RESET    Reset (not used))
Rx  0    to the tx of the nodeMcu
Gnd      ground
Raw: 5 to 16 VDC Max


Graphic LCD Pinout:
Micro Pro____ Mini Pro______LCD_______________
16				    11		      	LCD SDA .... Pin 6
15					  13		      	LCD SCK .... Pin 7
10					  10		  			LCD TFT CS.. Pin 3
9					    9	    	  		LCD A0	.... Pin 5
8				     	8				  	  LCD RST .... Pin 4
										        LCD Gnd .... Pin 1
										        LCD Vcc .... Pin 2  5 volts  from the minipro VCC
										        LCD Led To Vcc thru a resistor around 100 ohms


Cable wiring between nodeMcu and Mini Pro
-----------------------------------------
--NodeMcu--   --Mini/Micro Pro--
Tx            Rx
Gnd           Gnd
VU            Raw

Software:
ucglib must be installed in  arduino IDE library;
https://github.com/olikraus/ucglib


--------
WARNING:
The webradio serial must be set at 28800 b/s
--------
WARNING:
---------
The nodeMcu is a 3.3v device.
DO NOT connect the Rx pin of the lcd to the Tx pin of the mini Pro if it's a 5V 16Mhz


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
**** Instruction for the IR module *****************************************************************************************
------------------------------------
The model used here is from:
https://www.aliexpress.com/item/Hot-Selling-1pcs-New-Infrared-IR-Wireless-Remote-Control-Module-Kits-For-Arduino-Wholesale/32334118062.html?spm=2114.13010608.0.0.XzmgYk
or equivalent. Cost less than 1$

- Based on NEC protocol; Built-in 1 x AG10 battery;
- Remote control range: above 8m;
- Wavelength: 940Nm;
- Frequency: crystal oscillator: 455KHz; IR carrier frequency: 38KHz


// IR library
Install the library from https://github.com/cyborg5/IRLib2
Uncomment the #define IR  and the #ifdef IRLib2 at the beginning of the karadioU8glib.ino file.
----------------------

The IR receiver pins:
From left to right (pin at bottom, sensor in front of you)
Gnd VCC Signal

The signal must be connected to
For mini #define PIN_IRRECV  2  or 3 
For micro #define PIN_IRRECV  2  or 3 or 7 


In karadioUcglig.ino:
//  Uncomment the following line to see the code of your remote control and report to the case the value
//	    Serial.print("Protocol:");Serial.print(results.decode_type);Serial.print("  value:");Serial.println(results.value,HEX);
If you want to use another remote, you can see the code of the keys and modify the switch case for the needed function.
See IRremote.h  to add the protocol you need for the remote.

The tx of the pro mini must be connected to the rx of the esp12 but
Warning; the esp rx pin is not 5V tolerant, so if your pro mini is a 5v version we need to adapt the level.
-------
See
http://www.instructables.com/id/Pi-Cubed-How-to-connect-a-33V-Raspberry-Pi-to-a-5V/

The value of the resistors is not critical. Only the ration 1/3 2/3 must be respected 1k and 2k for example)

Enjoy.

jpc 01/2017
*/

