/*
 * Copyright 2016 Piotr Sperka (http://www.piotrsperka.info)
*/
#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "c_types.h"

// Comment line below if you don't want to use external SRAM
//#define USE_EXTERNAL_SRAM 1

extern bool externram;
uint32_t BUFFER_SIZE;

uint32_t getBufferFree();
uint32_t getBufferFilled();
uint32_t bufferWrite(uint8_t *data, uint32_t size);
uint32_t bufferRead(uint8_t *data, uint32_t size);
void bufferReset();

#endif
