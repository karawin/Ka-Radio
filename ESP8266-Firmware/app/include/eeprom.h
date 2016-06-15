#pragma once

#include "c_types.h"

struct device_settings {
	uint8_t dhcpEn;
	uint8_t ipAddr[4];
	uint8_t mask[4];
	uint8_t gate[4];
	char ssid[64];
	char pass[64];
	uint8_t vol;
	int8_t treble;
	uint8_t bass;
	int8_t freqtreble;
	uint8_t freqbass;
	uint8_t spacial;
	uint8_t future[109];
};

struct shoutcast_info {
	char domain[73]; //url
	char file[116];  //path
	char name[65];
	uint16_t port;	//port
};

uint8_t eeGetByte(uint32_t address);
void eeSetByte(uint32_t address, uint8_t data);
uint32_t eeGet4Byte(uint32_t address);
void eeSet4Byte(uint32_t address, uint32_t data);
void eeGetData(int address, void* buffer, int size);
void eeSetData(int address, void* buffer, int size);

void saveStation(struct shoutcast_info *station, uint8_t position);
void eeEraseStations();
struct shoutcast_info* getStation(uint8_t position);
void saveDeviceSettings(struct device_settings *settings);
struct device_settings* getDeviceSettings();
