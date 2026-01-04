#include "dri_touch_gt9xx.h"

#include "boa_touch.h"
#include "dri_i2c2.h"

/**
 * @file dri_touch_gt9xx.c
 * @brief drivers 层：GT9xx/GT615 触摸芯片的 I2C 总线与复位时序装配实现
 * @author ssw12
 */

static uint16_t s_addr_7bit = DRI_TOUCH_GT9XX_ADDR_7BIT_5D;
static bool s_inited = false;

uint16_t dri_touch_gt9xx_addr_7bit(void) { return s_addr_7bit; }

static bool dri_touch_gt9xx_probe_addr(void)
{
  /*
   * 地址探测：
   * - Goodix 常见 7-bit 地址：0x5D / 0x14
   * - 地址通常由上电复位阶段 INT 的电平选择（不同模组可能相反）
   */
  if (dri_i2c2_is_device_ready(DRI_TOUCH_GT9XX_ADDR_7BIT_5D, 2, 20) == HAL_OK)
  {
    s_addr_7bit = DRI_TOUCH_GT9XX_ADDR_7BIT_5D;
    return true;
  }
  if (dri_i2c2_is_device_ready(DRI_TOUCH_GT9XX_ADDR_7BIT_14, 2, 20) == HAL_OK)
  {
    s_addr_7bit = DRI_TOUCH_GT9XX_ADDR_7BIT_14;
    return true;
  }

  /* 两个地址都不应答：保留默认值，供上层继续尝试 */
  s_addr_7bit = DRI_TOUCH_GT9XX_ADDR_7BIT_5D;
  return false;
}

static void dri_touch_gt9xx_reset_and_probe(bool int_high)
{
  boa_touch_reset_for_gt9xx(int_high);
  (void)dri_touch_gt9xx_probe_addr();
}

void dri_touch_gt9xx_init(void)
{
  (void)dri_i2c2_init();

  /*
   * 已初始化时做一次轻量级检查：
   * - 若当前地址不应答，则进行一次复位并复探测
   */
  if (s_inited && dri_i2c2_is_device_ready(s_addr_7bit, 1, 5) == HAL_OK)
  {
    return;
  }

  /*
   * 复位与地址选择：
   * - 先按 “INT=0” 方案复位（常见对应 0x5D）
   * - 若未探测到 ACK，再按 “INT=1” 方案复位再试一次
   */
  dri_touch_gt9xx_reset_and_probe(false);
  if (dri_i2c2_is_device_ready(s_addr_7bit, 1, 5) != HAL_OK)
  {
    dri_touch_gt9xx_reset_and_probe(true);
  }

  s_inited = true;
}

HAL_StatusTypeDef dri_touch_gt9xx_mem_read(uint16_t reg, uint8_t *buf,
                                           uint16_t len, uint32_t timeout_ms)
{
  dri_touch_gt9xx_init();
  return dri_i2c2_mem_read(s_addr_7bit, reg, I2C_MEMADD_SIZE_16BIT, buf, len,
                           timeout_ms);
}

HAL_StatusTypeDef dri_touch_gt9xx_mem_write(uint16_t reg, const uint8_t *buf,
                                            uint16_t len, uint32_t timeout_ms)
{
  dri_touch_gt9xx_init();
  return dri_i2c2_mem_write(s_addr_7bit, reg, I2C_MEMADD_SIZE_16BIT, buf, len,
                            timeout_ms);
}

HAL_StatusTypeDef dri_touch_gt9xx_write_u8(uint16_t reg, uint8_t v,
                                           uint32_t timeout_ms)
{
  return dri_touch_gt9xx_mem_write(reg, &v, 1, timeout_ms);
}
