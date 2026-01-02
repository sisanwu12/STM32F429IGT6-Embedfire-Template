#include "dri_i2c1.h"

#include <stdbool.h>

static I2C_HandleTypeDef hi2c1;
static bool s_inited = false;

HAL_StatusTypeDef dri_i2c1_init(void)
{
  if (s_inited)
  {
    return HAL_OK;
  }

  hi2c1.Instance = I2C1;

  /*
   * I2C 时钟：
   * - CTP（电容触摸）通常支持 100k/400k
   * - 为了提高兼容性（线长/上拉电阻不理想等场景），这里默认使用 100k
   */
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  HAL_StatusTypeDef st = HAL_I2C_Init(&hi2c1);
  if (st != HAL_OK)
  {
    return st;
  }

  s_inited = true;
  return HAL_OK;
}

I2C_HandleTypeDef *dri_i2c1_handle(void)
{
  return &hi2c1;
}

static uint16_t to_hal_addr(uint16_t dev_addr_7bit)
{
  return (uint16_t)(dev_addr_7bit << 1);
}

HAL_StatusTypeDef dri_i2c1_mem_read(uint16_t dev_addr_7bit, uint16_t mem_addr,
                                    uint16_t mem_addr_size, uint8_t *data,
                                    uint16_t len, uint32_t timeout_ms)
{
  if (dri_i2c1_init() != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_I2C_Mem_Read(&hi2c1, to_hal_addr(dev_addr_7bit), mem_addr,
                          mem_addr_size, data, len, timeout_ms);
}

HAL_StatusTypeDef dri_i2c1_mem_write(uint16_t dev_addr_7bit, uint16_t mem_addr,
                                     uint16_t mem_addr_size,
                                     const uint8_t *data, uint16_t len,
                                     uint32_t timeout_ms)
{
  if (dri_i2c1_init() != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_I2C_Mem_Write(&hi2c1, to_hal_addr(dev_addr_7bit), mem_addr,
                           mem_addr_size, (uint8_t *)data, len, timeout_ms);
}

HAL_StatusTypeDef dri_i2c1_is_device_ready(uint16_t dev_addr_7bit,
                                          uint32_t trials,
                                          uint32_t timeout_ms)
{
  if (dri_i2c1_init() != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_I2C_IsDeviceReady(&hi2c1, to_hal_addr(dev_addr_7bit), trials,
                               timeout_ms);
}

HAL_StatusTypeDef dri_i2c1_write_read(uint16_t dev_addr_7bit,
                                     const uint8_t *wbuf, uint16_t wlen,
                                     uint8_t *rbuf, uint16_t rlen,
                                     uint32_t timeout_ms)
{
  if (dri_i2c1_init() != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_StatusTypeDef st =
      HAL_I2C_Master_Transmit(&hi2c1, to_hal_addr(dev_addr_7bit),
                              (uint8_t *)wbuf, wlen, timeout_ms);
  if (st != HAL_OK)
  {
    return st;
  }

  return HAL_I2C_Master_Receive(&hi2c1, to_hal_addr(dev_addr_7bit), rbuf, rlen,
                                timeout_ms);
}

HAL_StatusTypeDef dri_i2c1_write(uint16_t dev_addr_7bit, const uint8_t *wbuf,
                                uint16_t wlen, uint32_t timeout_ms)
{
  if (dri_i2c1_init() != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_I2C_Master_Transmit(&hi2c1, to_hal_addr(dev_addr_7bit),
                                 (uint8_t *)wbuf, wlen, timeout_ms);
}
