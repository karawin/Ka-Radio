/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Arduino Pro or Pro Mini w/ ATmega328 (5V, 16 MHz), Platform=avr, Package=arduino
*/

#define __AVR_ATmega328p__
#define __AVR_ATmega328P__
#define ARDUINO 106010
#define ARDUINO_MAIN
#define F_CPU 16000000L
#define __AVR__
#define F_CPU 16000000L
#define ARDUINO 106010
#define ARDUINO_AVR_PRO
#define ARDUINO_ARCH_AVR

void u8g_prepare(void);
void clearAll();
void cleartitle();
void eepromReadStr(int addr, char* str);
void eepromWriteStr(int addr, char* str);
void separator(char* from);
void parse(char* line);
void setup2();
void setup(void);
void serial();
void draw(int xx);
void scroll();
void loop(void);
int freeRam ();
void removeUtf8(byte *characters);

#include "pins_arduino.h" 
#include "arduino.h"
#include "karadioU8glib.ino"
#include "karadioutils.ino"
