/******************************************************************************
 * 
 * Copyright 2017 karawin (http://www.karawin.fr)
 * il ne faut pas decoder KaRadio
 *
*******************************************************************************/

#include "eeprom.h"
#include "flash.h"
#include "spi.h"
#include "stdio.h"
#include "stdlib.h"
#include "spi_flash.h"
#include <esp_libc.h>
#include "interface.h"

#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_RAM_ATTR __attribute__((section(".iram0.text")))

/*#define EEPROM_START	0x3F0000 // Last 64k of flash (32Mbits or 4 MBytes)
#define EEPROM_SIZE		0xBFFF	 // until xC000 (48k) espressif take the end
#define NBSTATIONS		192*/
/*
#ifdef ESP07
#define EEPROM_OLDSTART	0x0F0000 // Last 64k of flash (8Mbits or 1 MBytes)
#define EEPROM_START	0x0E0000 // Last 128k of flash (8Mbits or 1 MBytes)
#define EEPROM_START1	0x0D0000 // Last 128k of flash (8Mbits or 1 MBytes)
#else
#define EEPROM_OLDSTART	0x3F0000 // Last 64k of flash (32Mbits or 4 MBytes)
#define EEPROM_START	0x3E0000 // Last 128k of flash (32Mbits or 4 MBytes)
#define EEPROM_START1	0x3D0000 // Last 128k of flash (32Mbits or 4 MBytes)
#endif
*/
#define EEPROM_SIZE		0xFFFF	 // until xffff , 
#define NBOLDSTATIONS	192
#define NBSTATIONS		255

/*
struct device_settings device;
bool fdevice = false;
struct device_settings1 device1;
bool fdevice1 = false;
*/
const char streMSG[]  ICACHE_RODATA_ATTR STORE_ATTR  = {"Warning %s malloc low memory\n"};
const char saveStationPos[] ICACHE_RODATA_ATTR STORE_ATTR  = {"saveStation fails pos=%d\n"};
const char getStationPos[] ICACHE_RODATA_ATTR STORE_ATTR  = {"getStation fails pos=%d\n"};
const char streERASE[] ICACHE_RODATA_ATTR STORE_ATTR  = {"erase setting1 (only one time) \n"};
const char streGETDEVICE[] ICACHE_RODATA_ATTR STORE_ATTR  = {"getDeviceSetting%d fails\n"};
const char streSETDEVICE[] ICACHE_RODATA_ATTR STORE_ATTR  = {"saveDeviceSetting%d:  null\n"};

uint32_t Eeprom_start;
uint32_t Eeprom_start1;

uint32_t getFlashChipRealSize(void)
{
	uint32_t fSize = 1 << ((spi_flash_get_id() >> 16) & 0xFF);
	if (fSize > 0x400000) fSize = 0x400000;
	Eeprom_start = fSize - 0x20000;
	Eeprom_start1 = Eeprom_start - 0x10000;
	printf(PSTR("Eeprom_start: %x\nEeprom_start1: %x\n"),Eeprom_start,Eeprom_start1);
	
    return (1 << ((spi_flash_get_id() >> 16) & 0xFF));
}


ICACHE_FLASH_ATTR void eeGetDatax(uint32_t eeprom, int address, void* buffer, int size) { // address, size in BYTES !!!!
int result;
//printf("eeGetDatax, eeprom + address= %d, size= %d\n",eeprom + address,  size);
	result = spi_flash_read(eeprom + address, (uint32 *)buffer, size);
}

ICACHE_FLASH_ATTR void eeGetData(int address, void* buffer, int size) { // address, size in BYTES !!!!
//printf("eeGetData, address= %d, size= %d\n", address,  size);
	eeGetDatax(Eeprom_start,address,buffer,size);
}
ICACHE_FLASH_ATTR void eeGetData1(int address, void* buffer, int size) { // address, size in BYTES !!!!
	eeGetDatax(Eeprom_start1,address,buffer,size);
}

ICACHE_FLASH_ATTR bool eeSetDatax(uint32_t eeprom,int address, void* buffer, int size) { // address, size in BYTES !!!!
	uint8_t* inbuf = buffer;
int result;
uint32_t* eebuf= malloc(4096);
vTaskDelay(1); 
uint16_t i = 0;
if (eebuf != NULL)
{
	while(1) {
		
		uint32_t sector = (eeprom + address) & 0xFFF000;
		uint8_t* eebuf8 = (uint8_t*)eebuf;
		uint16_t startaddr = address & 0xFFF;
		uint16_t maxsize = 4096 - startaddr;
//printf("set1 startaddr: %x, size:%x, maxsize: %x, sector: %x, eebuf: %x\n",startaddr,size,maxsize,sector,eebuf);
//		spi_clock(HSPI, 4, 10); //2MHz
//		WRITE_PERI_REG(0x60000914, 0x73); //WDT clear
		spi_flash_read(sector, (uint32 *)eebuf, 4096);
		vTaskDelay(1);
		spi_flash_erase_sector(sector >> 12);		
		
//printf("set2 startaddr: %x, size:%x, maxsize: %x, sector: %x, eebuf: %x\n",startaddr,size,maxsize,sector,eebuf);
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
} else 
{
	printf(streMSG,"eebuf");/*heapSize();*/
	return false;
}
return true;
}

ICACHE_FLASH_ATTR bool eeSetData(int address, void* buffer, int size) { // address, size in BYTES !!!!
	return eeSetDatax(Eeprom_start, address, buffer, size);
}
ICACHE_FLASH_ATTR bool  eeSetData1(int address, void* buffer, int size) { // address, size in BYTES !!!!
	return eeSetDatax(Eeprom_start1, address, buffer, size);
}

ICACHE_FLASH_ATTR void eeSetClear(int address,uint8_t* eebuf) { // address, size in BYTES !!!!
		int i;
		spi_clock(HSPI, 4, 10); //2MHz
		WRITE_PERI_REG(0x60000914, 0x73); //WDT clear
		uint32_t sector = (Eeprom_start + address) & 0xFFF000;
		spi_flash_erase_sector(sector >> 12);
		for(i=0; i<4096; i++) eebuf[i] = 0;	
		spi_flash_write(sector, (uint32 *)eebuf, 4096);
}

ICACHE_FLASH_ATTR void eeSetClear1(int address,uint8_t* eebuf) { // address, size in BYTES !!!!
		int i;
		spi_clock(HSPI, 4, 10); //2MHz
		WRITE_PERI_REG(0x60000914, 0x73); //WDT clear
		uint32_t sector = (Eeprom_start1 + address) & 0xFFF000;
		spi_flash_erase_sector(sector >> 12);
		for(i=0; i<4096; i++) eebuf[i] = 0;	
		spi_flash_write(sector, (uint32 *)eebuf, 4096);
}

ICACHE_FLASH_ATTR void eeEraseAll() {
uint8_t* buffer= malloc(4096);
int i = 0;
//	printf("erase All\n");
	while (buffer == NULL) 
	{
		if (++i > 4) break;
		vTaskDelay(100); 
		buffer= malloc(4096); // last chance
	}	
	if (buffer != NULL)
	{
		for(i=0; i<4096; i++) buffer[i] = 0;			
		for(i=0; i<EEPROM_SIZE; i+=4096) {
//			printf("erase from %x \n",i);
			eeSetClear(i,buffer);
			vTaskDelay(1); // avoid watchdog
			eeSetClear1(i,buffer);
			vTaskDelay(1); // avoid watchdog
		}
//		printf("erase All done\n");
		free(buffer);
	}
}



ICACHE_FLASH_ATTR void eeErasesettings1(void) {
uint8_t* buffer= malloc(4096);
int i = 0;
	if (buffer != NULL)
	{
		for(i=0; i<4096; i++) buffer[i] = 0;
		printf(streERASE);		
		for(i=0; i<EEPROM_SIZE; i+=4096) {
//			printf("erase from %x \n",i);
			eeSetClear1(i,buffer);
			vTaskDelay(1); // avoid watchdog
		}
//		printf("erase All done\n");
		free(buffer);
	}
}

ICACHE_FLASH_ATTR void eeEraseStations() {
	uint8_t* buffer = malloc(4096);
	int i=0;
	while (buffer == NULL) 
	{
		if (++i > 10) break;
		vTaskDelay(10); 
		buffer= malloc(4096); // last chance
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
	} else printf(streMSG,"eeEraseStations");
}

ICACHE_FLASH_ATTR void saveStation(struct shoutcast_info *station, uint16_t position) {
	uint32_t i = 0;
	if (position > NBSTATIONS-1) {printf(saveStationPos,position); return;}
	while (!eeSetData((position+1)*256, station, 256)) 
	{
		kprintf(PSTR("Retrying %d on saveStation\n"),i+1);
		vTaskDelay ((i+1)*20+200) ;
		i++; 
		if (i == 2) {clientDisconnect(PSTR("saveStation low Memory")); vTaskDelay (300) ;} // stop the player
		if (i == 10) return;
	}
}
ICACHE_FLASH_ATTR void saveMultiStation(struct shoutcast_info *station, uint16_t position, uint8_t number) {
	uint32_t i = 0;
	while ((position +number-1) > NBSTATIONS-1) {printf(saveStationPos,position+number-1); number--; }
	if (number <= 0) return;
	while (!eeSetData((position+1)*256, station, number*256))
	{		
		kprintf(PSTR("Retrying %d on SaveMultiStation for %d stations\n"),i+1,number);
		vTaskDelay ((i+1)*20+300) ;
		i++; 
		if (i == 3) {clientDisconnect(PSTR("saveMultiStation low Memory")); vTaskDelay (300) ;}
		if (i == 10) return;
	}
}

/*ICACHE_FLASH_ATTR struct shoutcast_info* getOldStation(uint8_t position) {
	if (position > NBOLDSTATIONS-1) {printf("getOldStation pos=%d\n",position); return NULL;}
	uint8_t* buffer = malloc(256);
	while (buffer== NULL)
	{
		buffer = malloc(256);
        if ( buffer == NULL ){
			int i = 0;
			do { 
			i++;		
//			printf ("Heap size: %d\n",xPortGetFreeHeapSize( ));
			vTaskDelay(10);
			printf(PSTR("getOldstation fails for %d\n",256 ));
			}
			while (i<10);
			if (i >=10) {  return NULL;}
		} 		
	}
	eeGetOldData((position+1)*256, buffer, 256);
	return (struct shoutcast_info*)buffer;
}
*/
ICACHE_FLASH_ATTR struct shoutcast_info* getStation(uint8_t position) {
	if (position > NBSTATIONS-1) {printf(PSTR("getStation fails pos=%d\n"),position); return NULL;}
	uint8_t* buffer = malloc(256);
	uint8_t i = 0;
	
	while (buffer == NULL) 
	{
		printf(getStationPos,256 );
		if (++i > 2) break;
		vTaskDelay(400); 
		buffer= malloc(256); // last chance
	}	
	
	eeGetData((position+1)*256, buffer, 256);
	return (struct shoutcast_info*)buffer;
}

ICACHE_FLASH_ATTR void saveDeviceSettings(struct device_settings *settings) {
	if (settings == NULL) { printf(streSETDEVICE,0);return;}
//printf("saveDeviceSettings\n");
	eeSetData(0, settings, 256);
}
ICACHE_FLASH_ATTR void saveDeviceSettings1(struct device_settings1 *settings) {
	if (settings == NULL) { printf(streSETDEVICE,1);return;}
//printf("saveDeviceSettings1\n");
	eeSetData1(0, settings, 256);
}

/*ICACHE_FLASH_ATTR struct device_settings* getOldDeviceSettings() {
	uint8_t* buffer = malloc(256);
	if(buffer) {
		eeGetOldData(0, buffer, 256);
		return (struct device_settings*)buffer;
	} else { printf(PSTR("getOldDeviceSetting fails\n"));return NULL;}
}
*/



ICACHE_FLASH_ATTR struct device_settings* getDeviceSettingsSilent() {
	uint16_t size = 256;
	uint8_t* buffer = malloc(size);
	if(buffer) {	
		eeGetData(0, buffer, size);
		return (struct device_settings*)buffer;
	} 
	return NULL;
}

ICACHE_FLASH_ATTR struct device_settings* getDeviceSettings() {
	uint16_t size = 256;
	uint8_t* buffer = malloc(size);
	if(buffer) {	
		eeGetData(0, buffer, size);
		return (struct device_settings*)buffer;
	} else  printf(streGETDEVICE,0);
	return NULL;
}
ICACHE_FLASH_ATTR struct device_settings1* getDeviceSettings1() {
	uint16_t size = 256;
	uint8_t* buffer = malloc(size);
	if(buffer) {
		eeGetData1(0, buffer, size);
		return (struct device_settings1*)buffer;
	} else  printf(streGETDEVICE,1);
	return NULL;}