#pragma once

#include "c_types.h"
struct device_settings_ext {
	uint8_t dhcpEn2;
	uint8_t ipAddr2[4];
	uint8_t mask2[4];
	uint8_t gate2[4];	
	char ssid2[64]; 
	uint8_t pass2[64];
	uint8_t  futur[115];
};
struct device_settings {
	uint8_t dhcpEn;
	uint8_t ipAddr[4];
	uint8_t mask[4];
	uint8_t gate[4];
	char ssid[32]; 
	char ssid2[32]; 
	char pass[64];
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
	uint8_t theme;  // 0 ligth blue, 1 Dark brown
	char ua[40]; // user agent
	uint8_t pass2[60];
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

uint8_t eeGetByte(uint32_t address);
void eeSetByte(uint32_t address, uint8_t data);
uint32_t eeGet4Byte(uint32_t address);
void eeSet4Byte(uint32_t address, uint32_t data);
void eeGetOldData(int address, void* buffer, int size);
void eeSetOldData(int address, void* buffer, int size);
void eeGetData(int address, void* buffer, int size);
void eeSetData(int address, void* buffer, int size);

void saveStation(struct shoutcast_info *station, uint16_t position);
void saveMultiStation(struct shoutcast_info *station, uint16_t position, uint8_t number);
void eeEraseStations();
struct shoutcast_info* getStation(uint8_t position);
struct shoutcast_info* getOldStation(uint8_t position);
void saveDeviceSettings(struct device_settings *settings);
struct device_settings* getDeviceSettings();
struct device_settings* getOldDeviceSettings();
