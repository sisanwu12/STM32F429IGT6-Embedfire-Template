#pragma once

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * drivers/ 层：I2C1（片上外设）驱动
 *
 * 说明：
 * - 这里只封装 I2C 外设能力（init + mem read/write）
 * - 具体引脚映射（PB6/PB7 等）由 board/ 层的 HAL_I2C_MspInit 负责
 */

HAL_StatusTypeDef dri_i2c1_init(void);
I2C_HandleTypeDef *dri_i2c1_handle(void);

HAL_StatusTypeDef dri_i2c1_mem_read(uint16_t dev_addr_7bit, uint16_t mem_addr,
                                    uint16_t mem_addr_size, uint8_t *data,
                                    uint16_t len, uint32_t timeout_ms);
HAL_StatusTypeDef dri_i2c1_mem_write(uint16_t dev_addr_7bit, uint16_t mem_addr,
                                     uint16_t mem_addr_size,
                                     const uint8_t *data, uint16_t len,
                                     uint32_t timeout_ms);

#ifdef __cplusplus
} /*extern "C"*/
#endif

