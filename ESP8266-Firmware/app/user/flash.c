#include "flash.h"
#include "esp_common.h"



ICACHE_FLASH_ATTR uint32_t flashRead( void *to, uint32_t fromaddr, uint32_t size )
{
  fromaddr -= INTERNAL_FLASH_START_ADDRESS;
  int r;
  WRITE_PERI_REG(0x60000914, 0x73);
  r = spi_flash_read(fromaddr, (uint32 *)to, size);
  if(0 == r)
    return size;
  else{
    return 0;
  }
}

ICACHE_FLASH_ATTR uint32_t flashWrite( void *data, uint32_t fromaddr, uint32_t size )
{
  fromaddr -= INTERNAL_FLASH_START_ADDRESS;
  int r;
  WRITE_PERI_REG(0x60000914, 0x73);
  r = spi_flash_write(fromaddr, (uint32 *)data, size);
  if(0 == r)
    return size;
  else{
    return 0;
  }
}
