#include "dri_time_us.h"

#include "core_cm4.h"

static uint8_t s_inited = 0;
static uint32_t s_cycles_per_us = 0;

HAL_StatusTypeDef dri_time_us_init(void)
{
  if (s_inited)
  {
    return HAL_OK;
  }

  if (SystemCoreClock == 0u)
  {
    return HAL_ERROR;
  }

  s_cycles_per_us = SystemCoreClock / 1000000u;
  if (s_cycles_per_us == 0u)
  {
    return HAL_ERROR;
  }

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0u;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  s_inited = 1u;
  return HAL_OK;
}

uint32_t dri_time_cycles_now(void)
{
  (void)dri_time_us_init();
  return DWT->CYCCNT;
}

uint32_t dri_time_cycles_elapsed_us(uint32_t start_cycles)
{
  (void)dri_time_us_init();

  uint32_t now = DWT->CYCCNT;
  uint32_t diff = now - start_cycles; /* unsigned wrap OK */
  return diff / s_cycles_per_us;
}

void dri_time_delay_us(uint32_t us)
{
  (void)dri_time_us_init();

  uint32_t start = DWT->CYCCNT;
  uint32_t wait_cycles = us * s_cycles_per_us;
  while ((uint32_t)(DWT->CYCCNT - start) < wait_cycles)
  {
  }
}

