#include "dev_touch.h"

#include "boa_touch.h"
#include "dev_lcd.h"
#include "dri_i2c1.h"
#include "dri_i2c2.h"

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
static uint8_t s_last_err = 0;
static uint8_t s_ready_mask = 0;
static uint8_t s_bus_id = 0; /* 1:I2C1, 2:I2C2 */
static uint8_t s_last_status = 0;
static uint8_t s_last_int_active = 0;
static uint8_t s_last_int_level_high = 0;
static char s_product_id[5] = "----";
static uint8_t s_status_msb = 0;
static uint8_t s_status_lsb = 0;
static uint16_t s_cfg_x = 0;
static uint16_t s_cfg_y = 0;
static uint16_t s_cfg_x_msb = 0;
static uint16_t s_cfg_y_msb = 0;
static uint16_t s_cfg_x_lsb = 0;
static uint16_t s_cfg_y_lsb = 0;
static char s_i2c_scan[64] = "-";
static uint8_t s_rst_low_ready_mask = 0;
static char s_pid_hex_msb[9] = "--------";
static char s_pid_hex_lsb[9] = "--------";
static char s_cfg_hex_msb[11] = "----------";
static char s_cfg_hex_lsb[11] = "----------";
static char s_r40_hex[33] = "-";
static char s_r4e_hex[33] = "-";
static char s_8140_hex[33] = "-";
static char s_8150_hex[33] = "-";

/* 用于探测“触摸时到底哪个寄存器块在变化” */
#define DEV_TOUCH_SCAN_START 0x8100u
#define DEV_TOUCH_SCAN_SIZE 0x0100u
#define DEV_TOUCH_SCAN_BLOCK 16u
#define DEV_TOUCH_SCAN_BLOCKS (DEV_TOUCH_SCAN_SIZE / DEV_TOUCH_SCAN_BLOCK)
static uint8_t s_scan_baseline_xor[DEV_TOUCH_SCAN_BLOCKS] = {0};
static bool s_scan_baseline_ready = false;
static uint8_t s_scan_block_idx = 0;
static uint16_t s_scan_last_diff_addr = 0;
static char s_scan_last_diff_hex[33] = "-";

uint16_t dev_touch_debug_addr_7bit(void)
{
  return s_addr_7bit;
}

uint8_t dev_touch_debug_bus_id(void)
{
  return s_bus_id;
}

uint8_t dev_touch_debug_last_status(void)
{
  return s_last_status;
}

uint8_t dev_touch_debug_int_active(void)
{
  return s_last_int_active;
}

uint8_t dev_touch_debug_int_level_high(void)
{
  return s_last_int_level_high;
}

const char *dev_touch_debug_product_id(void)
{
  return s_product_id;
}

uint8_t dev_touch_debug_last_err(void)
{
  return s_last_err;
}

uint8_t dev_touch_debug_i2c_ready_mask(void)
{
  return s_ready_mask;
}

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

typedef enum
{
  /* HAL_I2C_Mem_Read 固定按 “高字节在前” 发送 16-bit mem_addr */
  GT911_REG_ADDR_MSB_FIRST = 0,
  /* 通过 swap16(reg) 让总线上发送 “低字节在前” */
  GT911_REG_ADDR_LSB_FIRST = 1,
} gt911_reg_addr_order_t;

/* Goodix 触摸常见为“低字节在前”，因此默认先用 LSB-first */
static gt911_reg_addr_order_t s_reg_order = GT911_REG_ADDR_LSB_FIRST;

uint8_t dev_touch_debug_reg_order(void)
{
  return (uint8_t)s_reg_order;
}

uint8_t dev_touch_debug_status_msb(void)
{
  return s_status_msb;
}

uint8_t dev_touch_debug_status_lsb(void)
{
  return s_status_lsb;
}

uint16_t dev_touch_debug_cfg_x(void)
{
  return s_cfg_x;
}

uint16_t dev_touch_debug_cfg_y(void)
{
  return s_cfg_y;
}

uint16_t dev_touch_debug_cfg_x_msb(void)
{
  return s_cfg_x_msb;
}

uint16_t dev_touch_debug_cfg_y_msb(void)
{
  return s_cfg_y_msb;
}

uint16_t dev_touch_debug_cfg_x_lsb(void)
{
  return s_cfg_x_lsb;
}

uint16_t dev_touch_debug_cfg_y_lsb(void)
{
  return s_cfg_y_lsb;
}

const char *dev_touch_debug_pid_hex_msb(void)
{
  return s_pid_hex_msb;
}

const char *dev_touch_debug_pid_hex_lsb(void)
{
  return s_pid_hex_lsb;
}

const char *dev_touch_debug_cfg_hex_msb(void)
{
  return s_cfg_hex_msb;
}

const char *dev_touch_debug_cfg_hex_lsb(void)
{
  return s_cfg_hex_lsb;
}

const char *dev_touch_debug_r40_hex(void)
{
  return s_r40_hex;
}

const char *dev_touch_debug_r4e_hex(void)
{
  return s_r4e_hex;
}

const char *dev_touch_debug_8140_hex(void)
{
  return s_8140_hex;
}

const char *dev_touch_debug_8150_hex(void)
{
  return s_8150_hex;
}

uint16_t dev_touch_debug_diff_addr(void)
{
  return s_scan_last_diff_addr;
}

const char *dev_touch_debug_diff_hex(void)
{
  return s_scan_last_diff_hex;
}

const char *dev_touch_debug_i2c_scan(void)
{
  return s_i2c_scan;
}

uint8_t dev_touch_debug_rst_low_ready_mask(void)
{
  return s_rst_low_ready_mask;
}

static uint16_t swap16(uint16_t v)
{
  return (uint16_t)((uint16_t)(v << 8) | (uint16_t)(v >> 8));
}

static char hex_digit(uint8_t v)
{
  v &= 0x0Fu;
  return (v < 10u) ? (char)('0' + v) : (char)('A' + (v - 10u));
}

static void bytes_to_hex(char *out, size_t out_len, const uint8_t *in,
                         size_t in_len)
{
  if (out == NULL || out_len == 0)
  {
    return;
  }

  if (in == NULL || (in_len * 2u + 1u) > out_len)
  {
    out[0] = '-';
    if (out_len > 1)
      out[1] = '\0';
    return;
  }

  for (size_t i = 0; i < in_len; i++)
  {
    out[i * 2u + 0u] = hex_digit((uint8_t)(in[i] >> 4));
    out[i * 2u + 1u] = hex_digit((uint8_t)(in[i] & 0x0Fu));
  }
  out[in_len * 2u] = '\0';
}

static HAL_StatusTypeDef gt911_read_with_order(gt911_reg_addr_order_t order,
                                               uint16_t reg, uint8_t *buf,
                                               uint16_t len);

static uint8_t xor_bytes(const uint8_t *buf, size_t len)
{
  uint8_t x = 0;
  if (buf == NULL)
    return 0;
  for (size_t i = 0; i < len; i++)
  {
    x ^= buf[i];
  }
  return x;
}

static void scan_init_baseline(void)
{
  uint8_t block[DEV_TOUCH_SCAN_BLOCK] = {0};
  for (uint16_t i = 0; i < (uint16_t)DEV_TOUCH_SCAN_BLOCKS; i++)
  {
    uint16_t addr =
        (uint16_t)(DEV_TOUCH_SCAN_START + (uint16_t)i * DEV_TOUCH_SCAN_BLOCK);
    if (gt911_read_with_order(GT911_REG_ADDR_MSB_FIRST, addr, block,
                              sizeof(block)) == HAL_OK)
    {
      s_scan_baseline_xor[i] = xor_bytes(block, sizeof(block));
    }
    else
    {
      s_scan_baseline_xor[i] = 0;
    }
  }
  s_scan_baseline_ready = true;
  s_scan_block_idx = 0;
  s_scan_last_diff_addr = 0;
  bytes_to_hex(s_scan_last_diff_hex, sizeof(s_scan_last_diff_hex), NULL, 16);
}

static void scan_step_update_diff(void)
{
  if (!s_scan_baseline_ready)
  {
    return;
  }

  uint8_t block[DEV_TOUCH_SCAN_BLOCK] = {0};
  uint16_t idx = s_scan_block_idx;
  uint16_t addr =
      (uint16_t)(DEV_TOUCH_SCAN_START + idx * (uint16_t)DEV_TOUCH_SCAN_BLOCK);

  if (gt911_read_with_order(GT911_REG_ADDR_MSB_FIRST, addr, block,
                            sizeof(block)) == HAL_OK)
  {
    uint8_t x = xor_bytes(block, sizeof(block));
    if (x != s_scan_baseline_xor[idx])
    {
      s_scan_last_diff_addr = addr;
      bytes_to_hex(s_scan_last_diff_hex, sizeof(s_scan_last_diff_hex), block,
                   sizeof(block));
    }
  }

  s_scan_block_idx++;
  if (s_scan_block_idx >= (uint8_t)DEV_TOUCH_SCAN_BLOCKS)
    s_scan_block_idx = 0;
}

static HAL_StatusTypeDef gt911_read_with_order(gt911_reg_addr_order_t order,
                                               uint16_t reg, uint8_t *buf,
                                               uint16_t len)
{
  uint16_t mem_addr = (order == GT911_REG_ADDR_MSB_FIRST) ? reg : swap16(reg);
  if (s_bus_id == 2)
  {
    return dri_i2c2_mem_read(s_addr_7bit, mem_addr, I2C_MEMADD_SIZE_16BIT, buf,
                             len, 50);
  }
  return dri_i2c1_mem_read(s_addr_7bit, mem_addr, I2C_MEMADD_SIZE_16BIT, buf,
                           len, 50);
}

static HAL_StatusTypeDef gt911_read(uint16_t reg, uint8_t *buf, uint16_t len)
{
  return gt911_read_with_order(s_reg_order, reg, buf, len);
}

static HAL_StatusTypeDef gt911_write_u8(uint16_t reg, uint8_t v)
{
  uint16_t mem_addr =
      (s_reg_order == GT911_REG_ADDR_MSB_FIRST) ? reg : swap16(reg);

  if (s_bus_id == 2)
  {
    return dri_i2c2_mem_write(s_addr_7bit, mem_addr, I2C_MEMADD_SIZE_16BIT, &v,
                              1, 50);
  }
  return dri_i2c1_mem_write(s_addr_7bit, mem_addr, I2C_MEMADD_SIZE_16BIT, &v, 1,
                            50);
}

static HAL_StatusTypeDef gt911_write_u8_with_order(gt911_reg_addr_order_t order,
                                                   uint16_t reg, uint8_t v)
{
  uint16_t mem_addr = (order == GT911_REG_ADDR_MSB_FIRST) ? reg : swap16(reg);

  if (s_bus_id == 2)
  {
    return dri_i2c2_mem_write(s_addr_7bit, mem_addr, I2C_MEMADD_SIZE_16BIT, &v,
                              1, 50);
  }
  return dri_i2c1_mem_write(s_addr_7bit, mem_addr, I2C_MEMADD_SIZE_16BIT, &v, 1,
                            50);
}

static HAL_StatusTypeDef gt911_read_mem8(uint8_t reg8, uint8_t *buf,
                                         uint16_t len)
{
  if (s_bus_id == 2)
  {
    return dri_i2c2_mem_read(s_addr_7bit, (uint16_t)reg8, I2C_MEMADD_SIZE_8BIT,
                             buf, len, 50);
  }
  return dri_i2c1_mem_read(s_addr_7bit, (uint16_t)reg8, I2C_MEMADD_SIZE_8BIT,
                           buf, len, 50);
}

static bool gt911_read_cfg_xy_with_order(gt911_reg_addr_order_t order,
                                         uint16_t *x, uint16_t *y)
{
  if (x == NULL || y == NULL)
  {
    return false;
  }

  uint8_t buf[5] = {0};
  if (gt911_read_with_order(order, 0x8047u, buf, sizeof(buf)) != HAL_OK)
  {
    *x = 0;
    *y = 0;
    return false;
  }

  *x = (uint16_t)((uint16_t)buf[1] | ((uint16_t)buf[2] << 8));
  *y = (uint16_t)((uint16_t)buf[3] | ((uint16_t)buf[4] << 8));
  return true;
}

static bool gt911_detect_reg_order_by_status(void)
{
  /*
   * 当 INT 为“有效”（通常为低）但 status 读不到 data-ready 时，
   * 用 status 寄存器本身来判定地址字节序：
   * - 正确字节序下，触摸时 status 通常会出现 bit7=1 或点数>0
   */
  gt911_reg_addr_order_t saved = s_reg_order;

  uint8_t s_msb = 0, s_lsb = 0;
  s_reg_order = GT911_REG_ADDR_MSB_FIRST;
  HAL_StatusTypeDef st_msb = gt911_read(GT911_REG_STATUS, &s_msb, 1);
  s_reg_order = GT911_REG_ADDR_LSB_FIRST;
  HAL_StatusTypeDef st_lsb = gt911_read(GT911_REG_STATUS, &s_lsb, 1);

  /* 恢复，后续按判定结果再设置 */
  s_reg_order = saved;

  if (st_msb != HAL_OK && st_lsb != HAL_OK)
  {
    return false;
  }

  /* 优先选择“data-ready” */
  if (st_msb == HAL_OK && (s_msb & 0x80u))
  {
    s_reg_order = GT911_REG_ADDR_MSB_FIRST;
    return true;
  }
  if (st_lsb == HAL_OK && (s_lsb & 0x80u))
  {
    s_reg_order = GT911_REG_ADDR_LSB_FIRST;
    return true;
  }

  /* 次优：选择“点数>0” */
  if (st_msb == HAL_OK && (s_msb & 0x0Fu))
  {
    s_reg_order = GT911_REG_ADDR_MSB_FIRST;
    return true;
  }
  if (st_lsb == HAL_OK && (s_lsb & 0x0Fu))
  {
    s_reg_order = GT911_REG_ADDR_LSB_FIRST;
    return true;
  }

  /* 兜底：若两者不同，选择非 0/非 0xFF 的那一个 */
  if (st_msb == HAL_OK && st_lsb == HAL_OK && s_msb != s_lsb)
  {
    bool msb_plausible = (s_msb != 0x00u && s_msb != 0xFFu);
    bool lsb_plausible = (s_lsb != 0x00u && s_lsb != 0xFFu);
    if (msb_plausible && !lsb_plausible)
    {
      s_reg_order = GT911_REG_ADDR_MSB_FIRST;
      return true;
    }
    if (lsb_plausible && !msb_plausible)
    {
      s_reg_order = GT911_REG_ADDR_LSB_FIRST;
      return true;
    }
  }

  return false;
}

static uint8_t gt911_count_printable(const uint8_t *buf, size_t len)
{
  uint8_t cnt = 0;
  if (buf == NULL)
  {
    return 0;
  }
  for (size_t i = 0; i < len; i++)
  {
    if (is_printable_ascii(buf[i]))
    {
      cnt++;
    }
  }
  return cnt;
}

static bool gt911_try_read_product_id(uint16_t addr_7bit)
{
  uint8_t id[4] = {0};
  s_addr_7bit = addr_7bit;
  HAL_StatusTypeDef st = gt911_read(GT911_REG_PRODUCT_ID, id, sizeof(id));

  if (st != HAL_OK)
  {
    return false;
  }

  /* 保存一份可显示的 Product ID */
  for (size_t i = 0; i < sizeof(id); i++)
  {
    s_product_id[i] = is_printable_ascii(id[i]) ? (char)id[i] : '.';
  }
  s_product_id[4] = '\0';

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

  uint8_t printable_cnt = gt911_count_printable(id, sizeof(id));
  return (printable_cnt >= 2u);
}

static bool gt911_detect_reg_order(void)
{
  /*
   * 通过读取 Product ID 自动判断寄存器地址字节序：
   * - 正常 GT911 的 0x8140 附近通常能读到可打印 ASCII（如 "911"）
   * - 若字节序错了，读到的往往是随机/不可打印/全 0
   */
  gt911_reg_addr_order_t saved = s_reg_order;

  s_reg_order = GT911_REG_ADDR_LSB_FIRST;
  if (gt911_try_read_product_id(s_addr_7bit))
  {
    return true;
  }

  s_reg_order = GT911_REG_ADDR_MSB_FIRST;
  if (gt911_try_read_product_id(s_addr_7bit))
  {
    return true;
  }

  s_reg_order = saved;
  return false;
}

static uint8_t gt911_score_cfg(uint16_t x, uint16_t y)
{
  if (x == 0u || y == 0u)
  {
    return 0u;
  }

  if (x > 4096u || y > 4096u)
  {
    return 0u;
  }

  uint16_t w = (uint16_t)dev_lcd_width();
  uint16_t h = (uint16_t)dev_lcd_height();

  bool match_wh = (x == w && y == h);
  bool match_hw = (x == h && y == w);

  if (match_wh || match_hw)
  {
    return 80u;
  }

  /* 兜底：只要在合理范围，也给一点分数用于“字节序二选一” */
  return 10u;
}

static void gt911_update_cfg_debug_and_maybe_fix_order(void)
{
  uint8_t cfg_msb[5] = {0};
  uint8_t cfg_lsb[5] = {0};

  bool ok_msb =
      (gt911_read_with_order(GT911_REG_ADDR_MSB_FIRST, 0x8047u, cfg_msb,
                             sizeof(cfg_msb)) == HAL_OK);
  bool ok_lsb =
      (gt911_read_with_order(GT911_REG_ADDR_LSB_FIRST, 0x8047u, cfg_lsb,
                             sizeof(cfg_lsb)) == HAL_OK);

  bytes_to_hex(s_cfg_hex_msb, sizeof(s_cfg_hex_msb), cfg_msb, sizeof(cfg_msb));
  bytes_to_hex(s_cfg_hex_lsb, sizeof(s_cfg_hex_lsb), cfg_lsb, sizeof(cfg_lsb));

  uint16_t x_msb =
      ok_msb ? (uint16_t)((uint16_t)cfg_msb[1] | ((uint16_t)cfg_msb[2] << 8))
             : 0u;
  uint16_t y_msb =
      ok_msb ? (uint16_t)((uint16_t)cfg_msb[3] | ((uint16_t)cfg_msb[4] << 8))
             : 0u;
  uint16_t x_lsb =
      ok_lsb ? (uint16_t)((uint16_t)cfg_lsb[1] | ((uint16_t)cfg_lsb[2] << 8))
             : 0u;
  uint16_t y_lsb =
      ok_lsb ? (uint16_t)((uint16_t)cfg_lsb[3] | ((uint16_t)cfg_lsb[4] << 8))
             : 0u;

  s_cfg_x_msb = x_msb;
  s_cfg_y_msb = y_msb;
  s_cfg_x_lsb = x_lsb;
  s_cfg_y_lsb = y_lsb;

  uint8_t score_msb = ok_msb ? gt911_score_cfg(x_msb, y_msb) : 0u;
  uint8_t score_lsb = ok_lsb ? gt911_score_cfg(x_lsb, y_lsb) : 0u;

  /* 避免保留旧值导致误判：如果两种都是 0，就把 C 也置 0 */
  if (score_msb == 0u && score_lsb == 0u)
  {
    s_cfg_x = 0;
    s_cfg_y = 0;
    return;
  }

  if (score_msb >= score_lsb)
  {
    s_reg_order = GT911_REG_ADDR_MSB_FIRST;
    s_cfg_x = x_msb;
    s_cfg_y = y_msb;
  }
  else
  {
    s_reg_order = GT911_REG_ADDR_LSB_FIRST;
    s_cfg_x = x_lsb;
    s_cfg_y = y_lsb;
  }
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

static HAL_StatusTypeDef i2c_is_ready_on_bus(uint8_t bus_id,
                                             uint16_t addr_7bit)
{
  if (bus_id == 2u)
  {
    return dri_i2c2_is_device_ready(addr_7bit, 1, 3);
  }
  return dri_i2c1_is_device_ready(addr_7bit, 1, 3);
}

static void i2c_scan_selected_bus(void);

static uint8_t gt911_ready_mask_on_bus(void)
{
  uint8_t mask = 0;
  if (i2c_is_ready_on_bus(s_bus_id, DEV_TOUCH_GT911_ADDR_7BIT_A) == HAL_OK)
    mask |= 0x01u;
  if (i2c_is_ready_on_bus(s_bus_id, DEV_TOUCH_GT911_ADDR_7BIT_B) == HAL_OK)
    mask |= 0x02u;
  return mask;
}

typedef struct
{
  bool int_high;
  uint8_t ready_mask;
  gt911_reg_addr_order_t reg_order;
  uint16_t cfg_x;
  uint16_t cfg_y;
  uint8_t cfg_score;
} gt911_probe_t;

static void gt911_probe_with_int_level(gt911_probe_t *out, bool int_high)
{
  if (out == NULL)
  {
    return;
  }

  out->int_high = int_high;
  out->ready_mask = 0;
  out->reg_order = s_reg_order;
  out->cfg_x = 0;
  out->cfg_y = 0;
  out->cfg_score = 0;

  gt911_reset_with_int_level(int_high);

  out->ready_mask = gt911_ready_mask_on_bus();
  if (out->ready_mask == 0u)
  {
    return;
  }

  /* 选择一个可用地址来读 cfg：优先 0x14，其次 0x5D */
  if (out->ready_mask & 0x02u)
    s_addr_7bit = DEV_TOUCH_GT911_ADDR_7BIT_B;
  else
    s_addr_7bit = DEV_TOUCH_GT911_ADDR_7BIT_A;

  uint16_t x_msb = 0, y_msb = 0;
  uint16_t x_lsb = 0, y_lsb = 0;
  (void)gt911_read_cfg_xy_with_order(GT911_REG_ADDR_MSB_FIRST, &x_msb, &y_msb);
  (void)gt911_read_cfg_xy_with_order(GT911_REG_ADDR_LSB_FIRST, &x_lsb, &y_lsb);

  uint8_t score_msb = gt911_score_cfg(x_msb, y_msb);
  uint8_t score_lsb = gt911_score_cfg(x_lsb, y_lsb);

  if (score_msb == 0u && score_lsb == 0u)
  {
    return;
  }

  if (score_msb >= score_lsb)
  {
    out->reg_order = GT911_REG_ADDR_MSB_FIRST;
    out->cfg_x = x_msb;
    out->cfg_y = y_msb;
    out->cfg_score = score_msb;
  }
  else
  {
    out->reg_order = GT911_REG_ADDR_LSB_FIRST;
    out->cfg_x = x_lsb;
    out->cfg_y = y_lsb;
    out->cfg_score = score_lsb;
  }
}

static void gt911_finalize_addr_and_order(void)
{
  gt911_probe_t p0, p1;
  gt911_probe_with_int_level(&p0, false);
  gt911_probe_with_int_level(&p1, true);

  const gt911_probe_t *best = &p0;
  if (p1.cfg_score > p0.cfg_score)
  {
    best = &p1;
  }
  else if (p1.cfg_score == p0.cfg_score)
  {
    /* 分数相同：优先选择能看到 0x14 的那一组（多数 GT911 模组实际用 0x14） */
    if ((p1.ready_mask & 0x02u) && !(p0.ready_mask & 0x02u))
      best = &p1;
  }

  if (best->ready_mask != 0u)
  {
    s_ready_mask = best->ready_mask;
    s_reg_order = best->reg_order;
    s_cfg_x = best->cfg_x;
    s_cfg_y = best->cfg_y;
    s_addr_7bit =
        (best->ready_mask & 0x02u) ? DEV_TOUCH_GT911_ADDR_7BIT_B
                                   : DEV_TOUCH_GT911_ADDR_7BIT_A;

    /* 再做一次复位把地址“锁定”到 best 的 INT 电平选择结果 */
    gt911_reset_with_int_level(best->int_high);
    s_ready_mask = gt911_ready_mask_on_bus();
  }

  i2c_scan_selected_bus();
}

static void scan_append_addr(uint8_t addr_7bit, size_t *pos)
{
  if (pos == NULL)
  {
    return;
  }

  size_t p = *pos;
  if (p >= sizeof(s_i2c_scan) - 1u)
  {
    return;
  }

  if (p != 0u)
  {
    if (p + 1u >= sizeof(s_i2c_scan) - 1u)
    {
      return;
    }
    s_i2c_scan[p++] = ',';
  }

  if (p + 4u >= sizeof(s_i2c_scan) - 1u)
  {
    return;
  }
  s_i2c_scan[p++] = '0';
  s_i2c_scan[p++] = 'x';
  s_i2c_scan[p++] = hex_digit((uint8_t)(addr_7bit >> 4));
  s_i2c_scan[p++] = hex_digit((uint8_t)(addr_7bit & 0x0Fu));
  s_i2c_scan[p] = '\0';
  *pos = p;
}

static void i2c_scan_selected_bus(void)
{
  size_t pos = 0;
  s_i2c_scan[0] = '\0';

  for (uint16_t a = 0x08u; a <= 0x77u; a++)
  {
    if (i2c_is_ready_on_bus(s_bus_id, a) != HAL_OK)
    {
      continue;
    }

    scan_append_addr((uint8_t)a, &pos);
    if (pos >= sizeof(s_i2c_scan) - 6u)
      break;
  }

  if (pos == 0)
  {
    s_i2c_scan[0] = '-';
    s_i2c_scan[1] = '\0';
  }
}

bool dev_touch_init(void)
{
  if (s_inited)
  {
    return true;
  }

  /* 1) 初始化 I2C（优先 I2C1，不通再尝试 I2C2） */
  if (dri_i2c1_init() == HAL_OK)
    s_bus_id = 1;
  else if (dri_i2c2_init() == HAL_OK)
    s_bus_id = 2;
  else
  {
    s_last_err = 1;
    s_bus_id = 0;
    return false;
  }

  /*
   * 2) 复位 + 地址探测
   *
   * 经验上“触摸完全没反应”的根因经常是复位阶段 INT 电平没拉对，
   * 导致地址没选中 / 进入了非 I2C 模式。
   */
  bool id_ok = false;
  bool addr_found = false;
  s_rst_low_ready_mask = 0;
  for (uint32_t bus_try = 0; bus_try < 2u && !id_ok; bus_try++)
  {
    /* 第一次用 I2C1，第二次用 I2C2（如果可用） */
    if (bus_try == 0u)
    {
      s_bus_id = 1;
      (void)dri_i2c1_init();
    }
    else
    {
      if (dri_i2c2_init() != HAL_OK)
      {
        continue;
      }
      s_bus_id = 2;
    }

    for (uint32_t pass = 0; pass < 2u && !id_ok; pass++)
    {
      gt911_reset_with_int_level(pass == 0u ? false : true);

      /* 记录两种地址在当前硬件上的 ACK 情况 */
      s_ready_mask = 0;
      if (s_bus_id == 2)
      {
        if (dri_i2c2_is_device_ready(DEV_TOUCH_GT911_ADDR_7BIT_A, 2, 20) == HAL_OK)
          s_ready_mask |= 0x01u;
        if (dri_i2c2_is_device_ready(DEV_TOUCH_GT911_ADDR_7BIT_B, 2, 20) == HAL_OK)
          s_ready_mask |= 0x02u;
      }
      else
      {
        if (dri_i2c1_is_device_ready(DEV_TOUCH_GT911_ADDR_7BIT_A, 2, 20) == HAL_OK)
          s_ready_mask |= 0x01u;
        if (dri_i2c1_is_device_ready(DEV_TOUCH_GT911_ADDR_7BIT_B, 2, 20) == HAL_OK)
          s_ready_mask |= 0x02u;
      }

      /* 只要有 ACK，先把地址选中，避免后续读 status 用错地址导致直接失败 */
      if (s_ready_mask != 0)
      {
        addr_found = true;
        s_addr_7bit = (s_ready_mask & 0x01u) ? DEV_TOUCH_GT911_ADDR_7BIT_A
                                             : DEV_TOUCH_GT911_ADDR_7BIT_B;
      }

      /* 优先用“能读出包含 '9' 的 ProductID”来确认寄存器字节序 */
      if (s_ready_mask & 0x01u)
      {
        s_addr_7bit = DEV_TOUCH_GT911_ADDR_7BIT_A;
        id_ok = gt911_detect_reg_order();
      }
      if (!id_ok && (s_ready_mask & 0x02u))
      {
        s_addr_7bit = DEV_TOUCH_GT911_ADDR_7BIT_B;
        id_ok = gt911_detect_reg_order();
      }
    }
  }

  if (!addr_found)
  {
    /* 未探测到可用地址：仍允许继续运行，但触摸可能读不到 */
    s_addr_7bit = DEV_TOUCH_GT911_ADDR_7BIT_A;
    s_last_err = 2;
  }
  else
  {
    /*
     * 已找到应答地址，但若无法确认寄存器字节序/有效 ProductID，
     * 后续在 `dev_touch_read` 中仍会尝试自适应切换字节序。
     */
    s_last_err = id_ok ? 0 : 5;
  }

  /*
   * 3) 做一次“RST 拉低”ACK 验证 + 用 cfg(800x480) 来最终确定：地址 + 寄存器字节序。
   *
   * - 如果 RST 拉低时 I2C 仍然 ACK，大概率说明：PI8 不是触控 IC 的复位脚，
   *   或者该模组对 reset/地址选择时序有特殊要求。
   */
  boa_touch_gpio_init();
  boa_touch_rst_set(false);
  HAL_Delay(5);
  s_rst_low_ready_mask = gt911_ready_mask_on_bus();
  boa_touch_rst_set(true);
  HAL_Delay(30);

  gt911_finalize_addr_and_order();
  scan_init_baseline();

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

  /* 先读 INT 状态，便于做字节序自适应判断 */
  s_last_int_active = boa_touch_int_is_active() ? 1u : 0u;
  s_last_int_level_high = boa_touch_int_level_high() ? 1u : 0u;

  /*
   * 若 INT 一直为“有效”，但我们还读不到 data-ready，优先用 status 来探测字节序。
   * 这比 ProductID 更可靠（部分模组的 ProductID 可能不是 '911'）。
   */
  if (s_last_int_active)
  {
    (void)gt911_detect_reg_order_by_status();
  }

  /* 采样两种字节序下的 status，便于现场判断 */
  (void)gt911_read_with_order(GT911_REG_ADDR_MSB_FIRST, GT911_REG_STATUS,
                              &s_status_msb, 1);
  (void)gt911_read_with_order(GT911_REG_ADDR_LSB_FIRST, GT911_REG_STATUS,
                              &s_status_lsb, 1);

  /* 采样 ProductID（两种字节序），用于确认寄存器地址序 */
  uint8_t pid_msb[4] = {0};
  uint8_t pid_lsb[4] = {0};
  if (gt911_read_with_order(GT911_REG_ADDR_MSB_FIRST, GT911_REG_PRODUCT_ID,
                            pid_msb, sizeof(pid_msb)) == HAL_OK)
    bytes_to_hex(s_pid_hex_msb, sizeof(s_pid_hex_msb), pid_msb, sizeof(pid_msb));
  else
    bytes_to_hex(s_pid_hex_msb, sizeof(s_pid_hex_msb), NULL, sizeof(pid_msb));

  if (gt911_read_with_order(GT911_REG_ADDR_LSB_FIRST, GT911_REG_PRODUCT_ID,
                            pid_lsb, sizeof(pid_lsb)) == HAL_OK)
    bytes_to_hex(s_pid_hex_lsb, sizeof(s_pid_hex_lsb), pid_lsb, sizeof(pid_lsb));
  else
    bytes_to_hex(s_pid_hex_lsb, sizeof(s_pid_hex_lsb), NULL, sizeof(pid_lsb));

  /* 同时读取 cfg（两种字节序），并在读到合理值时自动纠正寄存器字节序 */
  gt911_update_cfg_debug_and_maybe_fix_order();

  /* 每次轮询推进一次扫描，用于定位“触摸时变化的寄存器块” */
  scan_step_update_diff();

  /* 读取 0x8140/0x8150 周边窗口（GT911 常见触摸数据区域），用于反推寄存器布局 */
  uint8_t w8140[16] = {0};
  uint8_t w8150[16] = {0};
  if (gt911_read_with_order(GT911_REG_ADDR_MSB_FIRST, 0x8140u, w8140,
                            sizeof(w8140)) == HAL_OK)
    bytes_to_hex(s_8140_hex, sizeof(s_8140_hex), w8140, sizeof(w8140));
  else
    bytes_to_hex(s_8140_hex, sizeof(s_8140_hex), NULL, sizeof(w8140));

  if (gt911_read_with_order(GT911_REG_ADDR_MSB_FIRST, 0x8150u, w8150,
                            sizeof(w8150)) == HAL_OK)
    bytes_to_hex(s_8150_hex, sizeof(s_8150_hex), w8150, sizeof(w8150));
  else
    bytes_to_hex(s_8150_hex, sizeof(s_8150_hex), NULL, sizeof(w8150));

  /* 额外读取 8-bit 地址空间的关键窗口（用于判断是否存在“只识别低 8 位地址”的情况） */
  uint8_t r40[16] = {0};
  uint8_t r4e[16] = {0};
  if (gt911_read_mem8(0x40u, r40, sizeof(r40)) == HAL_OK)
    bytes_to_hex(s_r40_hex, sizeof(s_r40_hex), r40, sizeof(r40));
  else
    bytes_to_hex(s_r40_hex, sizeof(s_r40_hex), NULL, sizeof(r40));

  if (gt911_read_mem8(0x4Eu, r4e, sizeof(r4e)) == HAL_OK)
    bytes_to_hex(s_r4e_hex, sizeof(s_r4e_hex), r4e, sizeof(r4e));
  else
    bytes_to_hex(s_r4e_hex, sizeof(s_r4e_hex), NULL, sizeof(r4e));

  uint8_t status = 0;
  HAL_StatusTypeDef st = gt911_read(GT911_REG_STATUS, &status, 1);

  if (st != HAL_OK)
  {
    *pressed = false;
    s_last_err = 3;
    s_last_status = 0xFF;
    return true;
  }

  s_last_status = status;

  /* bit7: 数据就绪；低 4bit: 触摸点数量 */
  if ((status & 0x80u) == 0u)
  {
    /*
     * 若 INT 一直为低，但 status 读不到 data-ready：
     * - 尝试用两种字节序写 0 清状态，看是否能释放 INT（用于定位“寄存器地址是否读错”）
     */
    if (s_last_int_active)
    {
      (void)gt911_write_u8_with_order(GT911_REG_ADDR_MSB_FIRST, GT911_REG_STATUS,
                                      0);
      (void)gt911_write_u8_with_order(GT911_REG_ADDR_LSB_FIRST, GT911_REG_STATUS,
                                      0);
    }

    *pressed = false;
    s_last_err = 0;
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
    HAL_StatusTypeDef stp = HAL_ERROR;
    stp = gt911_read(GT911_REG_POINT1, p, sizeof(p));

    if (stp == HAL_OK)
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
      s_last_err = 0;
    }
    else
    {
      *pressed = false;
      s_last_err = 4;
    }
  }

  /* 清除状态（写 0 表示“数据已处理”） */
  (void)gt911_write_u8(GT911_REG_STATUS, 0);

  return true;
}
