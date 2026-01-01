#include "dev_sdram.h"

#include "dri_sdram.h"

HAL_StatusTypeDef dev_sdram_init(void)
{
  return dri_sdram_init();
}

