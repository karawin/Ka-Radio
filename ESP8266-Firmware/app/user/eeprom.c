#include "eeprom.h"
#include "flash.h"
#include "stdio.h"
#include "stdlib.h"
#include "spi_flash.h"

#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_RAM_ATTR __attribute__((section(".iram0.text")))

#define EEPROM_START	0x3F0000 // Last 64k of flash (32Mbits or 4 MBytes)
#define EEPROM_SIZE		0xBFFF	 // until xC000 (48k) espressif take the end
#define NBSTATIONS		192

//uint32_t eebuf[1024];
/*
ICACHE_FLASH_ATTR uint8_t eeGetByte(uint32_t address) { // address = number of 1-byte parts from beginning
	uint8_t t = 0;
	spi_flash_read(EEPROM_START + address, (uint32 *)&t, 1);
	return t;
}

ICACHE_FLASH_ATTR void eeSetByte(uint32_t address, uint8_t data) {
	uint32_t addr = (EEPROM_START + address) & 0xFFF000;
	spi_flash_read(addr, (uint32 *)eebuf, 4096);
	spi_flash_erase_sector(addr >> 12);
	eebuf[address & 0xFFF] = data;
	spi_flash_write(addr, (uint32 *)eebuf, 4096);
}

ICACHE_FLASH_ATTR uint32_t eeGet4Byte(uint32_t address) { // address = number of 4-byte parts from beginning
	address *= 4;
	uint32_t t = 0;
	spi_flash_read(EEPROM_START + address, (uint32 *)&t, 4);
	return t;
}

ICACHE_FLASH_ATTR void eeSet4Byte(uint32_t address, uint32_t data) {
	address *= 4;
	uint32_t addr = (EEPROM_START + address) & 0xFFF000;
	spi_flash_read(addr, (uint32 *)eebuf, 4096);
	spi_flash_erase_sector(addr >> 12);
	eebuf[(address/4) & 0xFFF] = data;
	spi_flash_write(addr, (uint32 *)eebuf, 4096);
}
*/
ICACHE_FLASH_ATTR void eeGetData(int address, void* buffer, int size) { // address, size in BYTES !!!!
int result;
	result = spi_flash_read(EEPROM_START + address, (uint32 *)buffer, size);

}

ICACHE_FLASH_ATTR void eeSetData(int address, void* buffer, int size) { // address, size in BYTES !!!!
	uint8_t* inbuf = buffer;
int result;
uint32_t* eebuf= malloc(4096);
	if (eebuf != NULL)
	{
	while(1) {
		uint32_t sector = (EEPROM_START + address) & 0xFFF000;
		spi_flash_read(sector, (uint32 *)eebuf, 4096);
		spi_flash_erase_sector(sector >> 12);
		
		uint8_t* eebuf8 = (uint8_t*)eebuf;
		uint16_t startaddr = address & 0xFFF;
		uint16_t maxsize = 4096 - startaddr;
		uint16_t i;
		
		for(i=0; (i<size && i<maxsize); i++) eebuf8[i+startaddr] = inbuf[i];
		result = spi_flash_write(sector, (uint32 *)eebuf, 4096);
		if(maxsize >= size) break;
		
		address += i;
		inbuf += i;
		size -= i;
	}
	free (eebuf);
	} else printf("eebuf malloc fails\n");
}

ICACHE_FLASH_ATTR void eeEraseAll() {
	uint8_t* buffer = malloc(4096);
	int i;
	for(i=0; i<4096; i++) buffer[i] = 0;
	for(i=0; i<EEPROM_SIZE; i+=4096) {
		eeSetData(i, buffer, 4096);
	}
	free(buffer);
}
ICACHE_FLASH_ATTR void eeEraseStations() {
	uint8_t* buffer = malloc(256);
	int i,j;
	for(i=0; i<256; i++) buffer[i] = 0;
	for(j=0; j<NBSTATIONS; j++){
		eeSetData((j+1)*256, buffer, 256);
		vTaskDelay(1); // avoid watchdog
	}
	free(buffer);
}
ICACHE_FLASH_ATTR void saveStation(struct shoutcast_info *station, uint8_t position) {
	if (position > NBSTATIONS-1) {printf("saveStation fails position=%d\n",position); return;}
	eeSetData((position+1)*256, station, 256);
}

ICACHE_FLASH_ATTR struct shoutcast_info* getStation(uint8_t position) {
	if (position > NBSTATIONS-1) {printf("getStation fails position=%d\n",position); return NULL;}
	uint8_t* buffer = malloc(256);
	while (buffer== NULL)
	{
		buffer = malloc(256);
        if ( buffer == NULL ){
			int i = 0;
			do { 
			i++;		
			printf ("Heap size: %d\n",xPortGetFreeHeapSize( ));
			vTaskDelay(10);
			printf("getstation malloc fails for %d\n",256 );
			}
			while (i<10);
			if (i >=10) { /*free(string);*/ return NULL;}
		} 		
	}
	eeGetData((position+1)*256, buffer, 256);
	return (struct shoutcast_info*)buffer;
}

ICACHE_FLASH_ATTR void saveDeviceSettings(struct device_settings *settings) {
	if (settings == NULL) { printf("saveDeviceSetting fails: settings null\n");return;}
	eeSetData(0, settings, 256);
}

ICACHE_FLASH_ATTR struct device_settings* getDeviceSettings() {
	uint8_t* buffer = malloc(256);
	if(buffer) {
		eeGetData(0, buffer, 256);
		return (struct device_settings*)buffer;
	} else { printf("getDeviceSetting fails: malloc\n");return NULL;}
}
