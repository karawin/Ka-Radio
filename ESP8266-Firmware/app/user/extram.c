/*
 * Copyright 2016 Piotr Sperka (http://www.piotrsperka.info)
*/

#include "extram.h"
#include "esp_common.h"
#include "spi.h"
#include "buffer.h"
#include "interface.h"

void extramInit() {
	char test[]=  "FFFFFFFFFFFFFFFF";
	char testram[]= "0123456789ABCDEF";
	gpio16_output_conf();
	gpio16_output_set(1);
	externram = false;
	spi_clock(HSPI, 4, 10); //2MHz
	extramWrite(16, 0, testram);
	extramRead(16, 0, test);
	if (memcmp(test,testram,16) == 0) 
		externram = true;	
	printf(PSTR("\n=> extraram state: %d 0x%x %s\n"),externram,test[0],test );
}

uint32_t extramRead(uint32_t size, uint32_t address, uint8_t *buffer) {
	uint32_t i = 0;
	spi_take_semaphore();
	spi_clock(HSPI, 3, 2); //13MHz
	gpio16_output_set(0);
	SPIPutChar(0x03);
	SPIPutChar((address>>16)&0xFF);
	SPIPutChar((address>>8)&0xFF);
	SPIPutChar(address&0xFF);
	for(i = 0; i < size; i++) {
		buffer[i] = SPIGetChar();
	}
	gpio16_output_set(1);
	spi_clock(HSPI, 4, 10); //2MHz
	spi_give_semaphore();
	return i;
}

uint32_t extramWrite(uint32_t size, uint32_t address, uint8_t *data) {
	uint32_t i = 0;
	spi_take_semaphore();
	spi_clock(HSPI, 3, 2); //13MHz
	gpio16_output_set(0);
	SPIPutChar(0x02);
	SPIPutChar((address>>16)&0xFF);
	SPIPutChar((address>>8)&0xFF);
	SPIPutChar(address&0xFF);
	for(i = 0; i < size; i++) {
		SPIPutChar(data[i]);
	}
	gpio16_output_set(1);
	spi_clock(HSPI, 4, 10); //2MHz
	spi_give_semaphore();
	return i;
}

