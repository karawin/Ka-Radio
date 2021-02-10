#pragma once

#include "c_types.h"

#define HOSTLEN		24
//define for bit array in theme
#define T_THEME 	1
#define NT_THEME	0xFE
#define T_PATCH 	2
#define NT_PATCH	0xFD
#define T_LED		4
#define NT_LED		0xFB
#define T_LEDPOL	8
#define NT_LEDPOL	0xF7
#define T_LOGTEL	0x10
#define NT_LOGTEL	0xEF
#define T_PROTECT	0x20
#define NT_PROTECT	0xDF


#define APMODE		0
#define STA1		1
#define STA2		2
#define SSIDLEN		32
#define PASSLEN		64
#define HOSTLEN		24
#define USERAGLEN	39
struct device_settings {
	uint8_t dhcpEn;
	uint8_t ipAddr[4];
	uint8_t mask[4];
	uint8_t gate[4];
	char ssid[SSIDLEN]; 
	char ssid2[SSIDLEN]; 
	char pass[PASSLEN];
	uint8_t vol;
	int8_t treble;
	uint8_t bass;
	int8_t freqtreble;
	uint8_t freqbass;
	uint8_t spacial;
	uint16_t currentstation;  // 
	uint8_t autostart; // 0: stopped, 1: playing
	uint8_t i2sspeed; // 0 = 48kHz, 1 = 96kHz, 2 = 128kHz
	uint32_t uartspeed; // serial baud
	uint8_t options;  // bit0:0 theme ligth blue, 1 Dark brown, bit1: 0 patch load  1 no patch, bit2: O blink led  1 led on On play
	char ua[USERAGLEN]; // user agent
	int8_t tzoffset; //timezone offset
	uint8_t pass2[60];
};

struct device_settings1 {
	uint16_t cleared; 		// 0xAABB if initialized
	uint32_t sleepValue; 	//6
	uint32_t wakeValue;		//10
	uint8_t dhcpEn;			//11
	uint8_t ipAddr[4];		//15
	uint8_t mask[4];		//19
	uint8_t gate[4];		//23	
	uint8_t pass2[PASSLEN];
	char hostname[HOSTLEN];	
	uint8_t fill[169-HOSTLEN-1];
};

struct shoutcast_info {
	char domain[73]; //url
	char file[116];  //path
	char name[64];
	int8_t ovol; // offset volume
	uint16_t port;	//port
};

struct shoutcast_info_ext {
	uint8_t  futur[256];
};	

uint32_t getFlashChipRealSize(void);
uint8_t eeGetByte(uint32_t address);
void eeSetByte(uint32_t address, uint8_t data);
uint32_t eeGet4Byte(uint32_t address);
void eeSet4Byte(uint32_t address, uint32_t data);
//void eeGetOldData(int address, void* buffer, int size);
//void eeSetOldData(int address, void* buffer, int size);
void eeGetData(int address, void* buffer, int size);
bool eeSetData(int address, void* buffer, int size);
bool eeSetData1(int address, void* buffer, int size);
void eeErasesettings1(void);
void saveStation(struct shoutcast_info *station, uint16_t position);
void saveMultiStation(struct shoutcast_info *station, uint16_t position, uint8_t number);
void eeEraseStations(void);
struct shoutcast_info* getStation(uint8_t position);
//struct shoutcast_info* getOldStation(uint8_t position);
void saveDeviceSettings(struct device_settings *settings);
void saveDeviceSettings1(struct device_settings1 *settings);
struct device_settings* getDeviceSettings();
struct device_settings* getDeviceSettingsSilent();
struct device_settings1* getDeviceSettings1(void);


// Protect: html page is password protected.
void setProtect(bool);
bool getProtect();