#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @file dri_touch_gt9xx.h
 * @brief drivers 层：GT9xx/GT615 触摸芯片的 I2C 总线与复位时序装配
 * @author ssw12
 *
 * 分层说明：
 * - drivers/：负责“把片上外设能力 + 板级装配”组合成可被 devices/ 使用的能力
 * - 本模块依赖：
 *   - `dri_i2c2`（I2C2 片上外设驱动）
 *   - `boa_touch`（RST/INT 引脚与复位时序）
 *
 * devices/（如 dev_gt9xx）通过本模块完成：
 * - I2C 初始化
 * - 复位阶段地址选择 + ACK 探测（0x5D / 0x14）
 * - 16-bit 寄存器读写封装
 */

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* GT9xx 常见 7-bit 地址 */
#define DRI_TOUCH_GT9XX_ADDR_7BIT_5D 0x5Du
#define DRI_TOUCH_GT9XX_ADDR_7BIT_14 0x14u

  /* 初始化 I2C2 + 触摸复位 + 地址探测（幂等） */
  void dri_touch_gt9xx_init(void);

  /* 返回当前探测到的 7-bit I2C 地址（默认 0x5D） */
  uint16_t dri_touch_gt9xx_addr_7bit(void);

  /* 16-bit 寄存器读写封装（内部会保证已 init & 已选择地址） */
  HAL_StatusTypeDef dri_touch_gt9xx_mem_read(uint16_t reg, uint8_t *buf,
                                             uint16_t len, uint32_t timeout_ms);
  HAL_StatusTypeDef dri_touch_gt9xx_mem_write(uint16_t reg, const uint8_t *buf,
                                              uint16_t len,
                                              uint32_t timeout_ms);
  HAL_StatusTypeDef dri_touch_gt9xx_write_u8(uint16_t reg, uint8_t v,
                                             uint32_t timeout_ms);

#ifdef __cplusplus
} /*extern "C"*/
#endif
