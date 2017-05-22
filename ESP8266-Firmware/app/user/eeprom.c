#include "eeprom.h"
#include "flash.h"
#include "stdio.h"
#include "stdlib.h"
#include "spi_flash.h"

#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_RAM_ATTR __attribute__((section(".iram0.text")))

/*#define EEPROM_START	0x3F0000 // Last 64k of flash (32Mbits or 4 MBytes)
#define EEPROM_SIZE		0xBFFF	 // until xC000 (48k) espressif take the end
#define NBSTATIONS		192*/
#define EEPROM_OLDSTART	0x3F0000 // Last 64k of flash (32Mbits or 4 MBytes)
#define EEPROM_START	0x3E0000 // Last 128k of flash (32Mbits or 4 MBytes)
#define EEPROM_START1	0x3D0000 // Last 128k of flash (32Mbits or 4 MBytes)
#define EEPROM_SIZE		0xFFFF	 // until xffff , 
#define NBOLDSTATIONS	192
#define NBSTATIONS		255

char msg[] = {"%s malloc fails\n"};

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

ICACHE_FLASH_ATTR void eeGetOldData(int address, void* buffer, int size) { // address, size in BYTES !!!!
int result;
	result = spi_flash_read(EEPROM_OLDSTART + address, (uint32 *)buffer, size);

}

ICACHE_FLASH_ATTR void eeSetOldData(int address, void* buffer, int size) { // address, size in BYTES !!!!
	uint8_t* inbuf = buffer;
int result;
uint32_t* eebuf= malloc(4096);
	if (eebuf != NULL)
	{
	while(1) {
		uint32_t sector = (EEPROM_OLDSTART + address) & 0xFFF000;
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
	} else printf(msg,"eebuf");
}


ICACHE_FLASH_ATTR void eeGetDatax(uint32_t eeprom, int address, void* buffer, int size) { // address, size in BYTES !!!!
int result;
	result = spi_flash_read(eeprom + address, (uint32 *)buffer, size);
}

ICACHE_FLASH_ATTR void eeGetData(int address, void* buffer, int size) { // address, size in BYTES !!!!
	eeGetDatax(EEPROM_START,address,buffer,size);
}
ICACHE_FLASH_ATTR void eeGetData1(int address, void* buffer, int size) { // address, size in BYTES !!!!
	eeGetDatax(EEPROM_START1,address,buffer,size);
}

ICACHE_FLASH_ATTR void eeSetDatax(uint32_t eeprom,int address, void* buffer, int size) { // address, size in BYTES !!!!
	uint8_t* inbuf = buffer;
int result;
uint32_t* eebuf= malloc(4096);
int i = 0;
	while (eebuf == NULL) 
	{
		vTaskDelay(200); 
		eebuf= malloc(4096); // last chance
		if (++i > 3) break;
	}	

	if (eebuf != NULL)
	{
	while(1) {
		uint32_t sector = (eeprom + address) & 0xFFF000;
		spi_flash_read(sector, (uint32 *)eebuf, 4096);
		spi_flash_erase_sector(sector >> 12);
		
		uint8_t* eebuf8 = (uint8_t*)eebuf;
		uint16_t startaddr = address & 0xFFF;
		uint16_t maxsize = 4096 - startaddr;
		uint16_t i;
//printf("set1 startaddr: %x, size:%x, maxsize: %x, sector: %x, eebuf: %x\n",startaddr,size,maxsize,sector,eebuf);
		for(i=0; (i<size && i<maxsize); i++) eebuf8[i+startaddr] = inbuf[i];
		result = spi_flash_write(sector, (uint32 *)eebuf, 4096);
//printf("set3 startaddr: %x, size:%x, maxsize: %x, result:%x, sector: %x, eebuf: %x\n",startaddr,size,maxsize,result,sector,eebuf);
		if(maxsize >= size) break;
		
		address += i;
		inbuf += i;
		size -= i;
//printf("set2 startaddr: %x, size:%x, maxsize: %x, sector: %x, eebuf: %x\n",startaddr,size,maxsize,sector,eebuf);
	}
	free (eebuf);
	} else printf(msg,"eebuf");
}

ICACHE_FLASH_ATTR void eeSetData(int address, void* buffer, int size) { // address, size in BYTES !!!!
	eeSetDatax(EEPROM_START, address, buffer, size);
}
ICACHE_FLASH_ATTR void eeSetData1(int address, void* buffer, int size) { // address, size in BYTES !!!!
	eeSetDatax(EEPROM_START1, address, buffer, size);
}

ICACHE_FLASH_ATTR void eeSetClear(int address,uint8_t* eebuf) { // address, size in BYTES !!!!
		int i;
		uint32_t sector = (EEPROM_START + address) & 0xFFF000;
		spi_flash_erase_sector(sector >> 12);
		for(i=0; i<4096; i++) eebuf[i] = 0;	
		spi_flash_write(sector, (uint32 *)eebuf, 4096);
		free (eebuf);
}

ICACHE_FLASH_ATTR void eeSetClear1(int address,uint8_t* eebuf) { // address, size in BYTES !!!!
		int i;
		uint32_t sector = (EEPROM_START1 + address) & 0xFFF000;
		spi_flash_erase_sector(sector >> 12);
		for(i=0; i<4096; i++) eebuf[i] = 0;	
		spi_flash_write(sector, (uint32 *)eebuf, 4096);
		free (eebuf);
}

ICACHE_FLASH_ATTR void eeEraseAll() {
uint8_t* buffer= malloc(4096);
int i = 0;
//	printf("erase All\n");
	while (buffer == NULL) 
	{
		vTaskDelay(10); 
		buffer= malloc(4096); // last chance
		if (++i > 10) break;
	}	
	if (buffer != NULL)
	{
		for(i=0; i<4096; i++) buffer[i] = 0;			
		for(i=0; i<EEPROM_SIZE; i+=4096) {
//			printf("erase from %x \n",i);
			eeSetClear(i,buffer);
//			eeSetClear1(i,buffer);
			vTaskDelay(1); // avoid watchdog
		}
//		printf("erase All done\n");
	}
}
ICACHE_FLASH_ATTR void eeEraseStations() {
	uint8_t* buffer = malloc(4096);
	int i=0;
	int j;
	while (buffer == NULL) 
	{
		vTaskDelay(10); 
		buffer= malloc(4096); // last chance
		if (++i > 10) break;
	}
	if (buffer != NULL) 
	{
		for(i=0; i<4096; i++) buffer[i] = 0;		
		eeSetData(256, buffer, 256*15); // 0xF00

		for (i=1;i<16;i++)
		{
//			printf("erase from %x \n",4096*i);
			eeSetClear(4096*i,buffer);
			vTaskDelay(1); // avoid watchdog
		}
		free(buffer);
	} else printf(msg,"eeEraseStations");
}

ICACHE_FLASH_ATTR void saveStation(struct shoutcast_info *station, uint16_t position) {
	if (position > NBSTATIONS-1) {printf("saveStation fails position=%d\n",position); return;}
	eeSetData((position+1)*256, station, 256);
}
ICACHE_FLASH_ATTR void saveMultiStation(struct shoutcast_info *station, uint16_t position, uint8_t number) {
	while ((position +number-1) > NBSTATIONS-1) {printf("saveStation fails position=%d\n",position+number-1); number--; }
	if (number <= 0) return;
	eeSetData((position+1)*256, station, number*256);
}

ICACHE_FLASH_ATTR struct shoutcast_info* getOldStation(uint8_t position) {
	if (position > NBOLDSTATIONS-1) {printf("getOldStation fails position=%d\n",position); return NULL;}
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
			printf("getOldstation malloc fails for %d\n",256 );
			}
			while (i<10);
			if (i >=10) { /*free(string);*/ return NULL;}
		} 		
	}
	eeGetOldData((position+1)*256, buffer, 256);
	return (struct shoutcast_info*)buffer;
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
ICACHE_FLASH_ATTR struct device_settings* getOldDeviceSettings() {
	uint8_t* buffer = malloc(256);
	if(buffer) {
		eeGetOldData(0, buffer, 256);
		return (struct device_settings*)buffer;
	} else { printf("getDeviceSetting fails: malloc\n");return NULL;}
}
ICACHE_FLASH_ATTR struct device_settings* getDeviceSettings() {
	uint8_t* buffer = malloc(256);
	if(buffer) {
		eeGetData(0, buffer, 256);
		return (struct device_settings*)buffer;
	} else { printf("getDeviceSetting fails: malloc\n");return NULL;}
}
