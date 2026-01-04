#include "gt9xx.h"

#include "bsp_i2c_touch.h"
#include "dri_i2c2.h"

#include "stm32f4xx_hal.h"

/* Goodix 坐标读取寄存器（官方库：GTP_READ_COOR_ADDR） */
#define GTP_READ_COOR_ADDR 0x814Eu

/* Product ID（可用于连通性校验） */
#define GTP_REG_VERSION 0x8140u

static HAL_StatusTypeDef gtp_mem_read(uint16_t reg, uint8_t *buf, uint16_t len)
{
  return dri_i2c2_mem_read(bsp_touch_addr_7bit(), reg, I2C_MEMADD_SIZE_16BIT, buf,
                           len, 50);
}

static HAL_StatusTypeDef gtp_mem_write_u8(uint16_t reg, uint8_t v)
{
  return dri_i2c2_mem_write(bsp_touch_addr_7bit(), reg, I2C_MEMADD_SIZE_16BIT, &v,
                            1, 50);
}

int32_t GTP_Init_Panel(void)
{
  I2C_Touch_Init();

  /*
   * 最小连通性校验：
   * - 读 0x8140（ProductID / Version 区域）若成功，基本说明 I2C + 地址 + 16-bit reg OK
   */
  uint8_t pid[4] = {0};
  if (gtp_mem_read(GTP_REG_VERSION, pid, sizeof(pid)) != HAL_OK)
  {
    return -1;
  }

  return 0;
}

static uint16_t le16(const uint8_t *p)
{
  return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

int GTP_Execu(int *x, int *y)
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
    /* 数据未就绪：清标志后退出，避免状态卡住 */
    (void)gtp_mem_write_u8(GTP_READ_COOR_ADDR, 0);
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

