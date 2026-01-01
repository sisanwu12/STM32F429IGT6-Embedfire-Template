#include "dev_touch.h"

#include "boa_touch.h"
#include "dev_lcd.h"
#include "dri_i2c1.h"

#include <stddef.h>

/*
 * GT911 I2C 地址（7-bit）：
 * - 常见为 0x5D 或 0x14
 * - 具体由 INT/RST 上电复位时序决定
 *
 * 为了“上电就能用”，这里做自动探测：
 * - 先尝试 0x5D，再尝试 0x14
 */
#define DEV_TOUCH_GT911_ADDR_7BIT_A 0x5Du
#define DEV_TOUCH_GT911_ADDR_7BIT_B 0x14u

/* GT911 关键寄存器 */
#define GT911_REG_PRODUCT_ID 0x8140u
#define GT911_REG_STATUS 0x814Eu
#define GT911_REG_POINT1 0x8150u

static bool s_inited = false;
static uint16_t s_addr_7bit = DEV_TOUCH_GT911_ADDR_7BIT_A;

/*
 * 触摸坐标变换开关：
 * - 不同模组可能会出现坐标轴交换/镜像（你会看到“点了按钮没反应”）
 * - 先用默认 0；如果屏幕上看到坐标变化但点不到按钮，再按现象打开这些宏
 */
#ifndef DEV_TOUCH_SWAP_XY
#define DEV_TOUCH_SWAP_XY 0
#endif
#ifndef DEV_TOUCH_INVERT_X
#define DEV_TOUCH_INVERT_X 0
#endif
#ifndef DEV_TOUCH_INVERT_Y
#define DEV_TOUCH_INVERT_Y 0
#endif

static bool is_printable_ascii(uint8_t c)
{
  return (c >= 0x20u && c <= 0x7Eu);
}

static bool gt911_try_read_product_id(uint16_t addr_7bit)
{
  uint8_t id[4] = {0};
  if (dri_i2c1_mem_read(addr_7bit, GT911_REG_PRODUCT_ID, I2C_MEMADD_SIZE_16BIT,
                        id, sizeof(id), 50) != HAL_OK)
  {
    return false;
  }

  /*
   * Product ID 常见为可打印 ASCII（例如 "911" 等），
   * 这里做一个宽松校验：避免全 0 / 全 0xFF 这种“总线悬空”假阳性。
   */
  uint8_t all0 = (uint8_t)(id[0] | id[1] | id[2] | id[3]);
  uint8_t all1 = (uint8_t)(id[0] & id[1] & id[2] & id[3]);
  if (all0 == 0x00u || all1 == 0xFFu)
  {
    return false;
  }

  uint8_t printable_cnt = 0;
  for (size_t i = 0; i < sizeof(id); i++)
  {
    if (is_printable_ascii(id[i]))
    {
      printable_cnt++;
    }
  }

  return (printable_cnt >= 2u);
}

static void gt911_reset_with_int_level(bool int_high)
{
  /*
   * GT911 需要在复位阶段通过 INT 引脚选择 I2C 地址/工作模式（常见做法）：
   * - 先把 INT 配置为推挽输出并拉到期望电平
   * - 进行 RST 脉冲
   * - 再把 INT 释放回输入
   *
   * 不同模组对“高/低选择哪个地址”的定义可能相反，所以我们后续会：
   * - INT=0 复位后探测 0x5D/0x14
   * - INT=1 复位后再探测 0x5D/0x14
   */
  boa_touch_int_write(int_high);
  boa_touch_reset_pulse();
  boa_touch_int_set_output(false);
  HAL_Delay(10);
}

bool dev_touch_init(void)
{
  if (s_inited)
  {
    return true;
  }

  /* 1) 初始化 I2C1（GPIO 复用在 boa_hal_msp.c 的 HAL_I2C_MspInit） */
  if (dri_i2c1_init() != HAL_OK)
  {
    return false;
  }

  /*
   * 2) 复位 + 地址探测
   *
   * 经验上“触摸完全没反应”的根因经常是复位阶段 INT 电平没拉对，
   * 导致地址没选中 / 进入了非 I2C 模式。
   */
  bool ok = false;
  for (uint32_t pass = 0; pass < 2u && !ok; pass++)
  {
    gt911_reset_with_int_level(pass == 0u ? false : true);

    if (gt911_try_read_product_id(DEV_TOUCH_GT911_ADDR_7BIT_A))
    {
      s_addr_7bit = DEV_TOUCH_GT911_ADDR_7BIT_A;
      ok = true;
    }
    else if (gt911_try_read_product_id(DEV_TOUCH_GT911_ADDR_7BIT_B))
    {
      s_addr_7bit = DEV_TOUCH_GT911_ADDR_7BIT_B;
      ok = true;
    }
  }

  if (!ok)
  {
    /* 未探测到可用地址：仍允许继续运行，但触摸可能读不到 */
    s_addr_7bit = DEV_TOUCH_GT911_ADDR_7BIT_A;
  }

  s_inited = true;
  return true;
}

bool dev_touch_read(bool *pressed, uint16_t *x, uint16_t *y)
{
  if (pressed == NULL || x == NULL || y == NULL)
  {
    return false;
  }

  if (!s_inited && !dev_touch_init())
  {
    *pressed = false;
    *x = 0;
    *y = 0;
    return true;
  }

  uint8_t status = 0;
  if (dri_i2c1_mem_read(s_addr_7bit, GT911_REG_STATUS, I2C_MEMADD_SIZE_16BIT,
                        &status, 1, 20) != HAL_OK)
  {
    *pressed = false;
    return true;
  }

  /* bit7: 数据就绪；低 4bit: 触摸点数量 */
  if ((status & 0x80u) == 0u)
  {
    *pressed = false;
    return true;
  }

  uint8_t point_num = (uint8_t)(status & 0x0Fu);
  if (point_num == 0u)
  {
    *pressed = false;
  }
  else
  {
    /* 只取第 1 个触摸点（单点） */
    uint8_t p[8] = {0};
    if (dri_i2c1_mem_read(s_addr_7bit, GT911_REG_POINT1,
                          I2C_MEMADD_SIZE_16BIT, p, sizeof(p), 30) == HAL_OK)
    {
      uint16_t tx = (uint16_t)((uint16_t)p[1] | ((uint16_t)p[2] << 8));
      uint16_t ty = (uint16_t)((uint16_t)p[3] | ((uint16_t)p[4] << 8));

#if DEV_TOUCH_SWAP_XY
      uint16_t tmp = tx;
      tx = ty;
      ty = tmp;
#endif
#if DEV_TOUCH_INVERT_X
      tx = (uint16_t)((uint16_t)dev_lcd_width() - 1u - tx);
#endif
#if DEV_TOUCH_INVERT_Y
      ty = (uint16_t)((uint16_t)dev_lcd_height() - 1u - ty);
#endif

      *pressed = true;
      *x = tx;
      *y = ty;
    }
    else
    {
      *pressed = false;
    }
  }

  /* 清除状态（写 0 表示“数据已处理”） */
  uint8_t clear = 0;
  (void)dri_i2c1_mem_write(s_addr_7bit, GT911_REG_STATUS,
                           I2C_MEMADD_SIZE_16BIT, &clear, 1, 20);

  return true;
}
