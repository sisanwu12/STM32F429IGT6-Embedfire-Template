#include "bsp_i2c_touch.h"

#include "boa_touch.h"
#include "dri_i2c2.h"

#include "stm32f4xx_hal.h"

static uint16_t s_addr_7bit = BSP_TOUCH_ADDR_7BIT_5D;
static bool s_inited = false;

uint16_t bsp_touch_addr_7bit(void)
{
  return s_addr_7bit;
}

void I2C_ResetChip(void)
{
  /*
   * 复位时序来自你验证可用的 touch/ 版本：
   * - 复位阶段让 INT 输出一个固定电平用于选择 I2C 地址
   * - RST 拉低/拉高并延时足够长
   * - 最后释放 INT 为输入
   *
   * 这里优先按 “INT=0 -> 0x5D” 的方案复位；后续会用 ACK 再确认一次地址。
   */
  boa_touch_reset_for_gt9xx(false);
}

void I2C_Touch_Init(void)
{
  if (s_inited)
  {
    return;
  }

  (void)dri_i2c2_init();

  I2C_ResetChip();

  /*
   * 地址探测：
   * - Goodix 常见 7-bit 地址：0x5D / 0x14
   * - 复位时序若改变，地址可能随之变化，因此这里做一次 ACK 选择
   */
  if (dri_i2c2_is_device_ready(BSP_TOUCH_ADDR_7BIT_5D, 2, 20) == HAL_OK)
  {
    s_addr_7bit = BSP_TOUCH_ADDR_7BIT_5D;
  }
  else if (dri_i2c2_is_device_ready(BSP_TOUCH_ADDR_7BIT_14, 2, 20) == HAL_OK)
  {
    s_addr_7bit = BSP_TOUCH_ADDR_7BIT_14;
  }
  else
  {
    /* 两个地址都不应答：保留默认值，供上层继续尝试 */
    s_addr_7bit = BSP_TOUCH_ADDR_7BIT_5D;
  }

  s_inited = true;
}

