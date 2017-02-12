/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Arduino/Genuino Micro, Platform=avr, Package=arduino
*/

#define __AVR_ATmega32u4__
#define __AVR_ATmega32U4__
#define ARDUINO 106013
#define ARDUINO_MAIN
#define F_CPU 16000000L
#define __AVR__
#define F_CPU 16000000L
#define ARDUINO 106013
#define ARDUINO_AVR_MICRO
#define ARDUINO_ARCH_AVR
#define USB_VID 0x2341
#define USB_PID 0x8037

void setup2(void);
void setup(void);
void clearAll();
void cleartitle();
void removeUtf8(byte *characters);
void eepromReadStr(int addr, char* str);
void eepromWriteStr(int addr, char* str);
void separator(char* from);
void parse(char* line);
void serial();
void drawFrame();
void setColor(int i);
void draw(int i);
void drawStringRoll(int index,ucg_int_t ix, ucg_int_t iy, uint8_t dir, const char *str);
void drawScroll(int i);
void scroll();
void nbStation(char nb);
void translateIR();
void translateIR();
void loop(void);

#include "pins_arduino.h" 
#include "arduino.h"
#include "karadioUCglib.ino"
