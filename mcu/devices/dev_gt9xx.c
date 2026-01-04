#include "dev_gt9xx.h"

#include "dri_touch_gt9xx.h"

#include <stddef.h>

/**
 * @file dev_gt9xx.c
 * @brief devices 层：Goodix GT9xx/GT615 最小轮询读取实现
 * @author ssw12
 *
 * 行为：
 * - 初始化时做一次最小 I2C 连通性校验（0x8140）
 * - 运行时读取 0x814E 的单点触摸并清状态
 */

/* Goodix 坐标读取寄存器（官方库：GTP_READ_COOR_ADDR） */
#define GTP_READ_COOR_ADDR 0x814Eu

/* Product ID（可用于连通性校验） */
#define GTP_REG_VERSION 0x8140u

#define GTP_I2C_TIMEOUT_MS 50u

static HAL_StatusTypeDef gtp_mem_read(uint16_t reg, uint8_t *buf, uint16_t len)
{
  return dri_touch_gt9xx_mem_read(reg, buf, len, GTP_I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef gtp_mem_write_u8(uint16_t reg, uint8_t v)
{
  return dri_touch_gt9xx_write_u8(reg, v, GTP_I2C_TIMEOUT_MS);
}

bool dev_gt9xx_init(void)
{
  dri_touch_gt9xx_init();

  /*
   * 最小连通性校验：
   * - 读 0x8140（ProductID / Version 区域）若成功，基本说明 I2C + 地址 + 16-bit
   * reg OK
   */
  uint8_t pid[4] = {0};
  if (gtp_mem_read(GTP_REG_VERSION, pid, sizeof(pid)) != HAL_OK)
  {
    return false;
  }

  return true;
}

static uint16_t le16(const uint8_t *p)
{
  return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

int dev_gt9xx_read(int *x, int *y)
{
  /*
   * 读取协议来自你验证可用的 touch/Src/gt9xx.c（GTP_Execu）：
   * - 从 0x814E 读取：status(1) + point0(8) + reserved(1) = 10 字节
   * - status:
   *     bit7=数据就绪
   *     bit0..3=触摸点数
   * - point0:
   *     [0]=track id
   *     [1..2]=x (LE)
   *     [3..4]=y (LE)
   */
  uint8_t data[10] = {0};
  if (gtp_mem_read(GTP_READ_COOR_ADDR, data, sizeof(data)) != HAL_OK)
  {
    return 0;
  }

  uint8_t finger = data[0];
  if (finger == 0x00u)
  {
    return 0;
  }

  if ((finger & 0x80u) == 0u)
  {
    /* 数据未就绪：保持寄存器状态，等待下一次轮询 */
    return 0;
  }

  uint8_t touch_num = (uint8_t)(finger & 0x0Fu);
  if (touch_num == 0u)
  {
    (void)gtp_mem_write_u8(GTP_READ_COOR_ADDR, 0);
    return 0;
  }

  if (x != NULL)
  {
    *x = (int)le16(&data[2]);
  }
  if (y != NULL)
  {
    *y = (int)le16(&data[4]);
  }

  /* 清空标志（写 0 表示“数据已处理”） */
  (void)gtp_mem_write_u8(GTP_READ_COOR_ADDR, 0);

  return (int)touch_num;
}
