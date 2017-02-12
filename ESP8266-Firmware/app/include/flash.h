#pragma once

#include "c_types.h"
#
#define INTERNAL_FLASH_START_ADDRESS    0x40200000

uint32_t flashRead( void *to, uint32_t fromaddr, uint32_t size );
uint32_t flashWrite( void *data, uint32_t fromaddr, uint32_t size );
