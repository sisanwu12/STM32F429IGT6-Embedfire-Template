#include "dev_touch.h"

#include "gt9xx.h"

#include "dev_lcd.h"

/*
 * 抛弃之前的“扫描/大段调试”逻辑：本文件只做一件事——把已验证可用的 touch/ 驱动
 * 封装成统一的 devices/dev_touch 接口，供 LVGL 输入设备使用。
 *
 * 触摸库核心行为：
 * - 初始化：I2C2 + GT9xx/GT615 复位时序（见 mcu/Libraries/touch）
 * - 读取：轮询 0x814E，解析单点触摸并清状态
 */

static bool s_inited = false;

/*
 * 触摸坐标变换开关：
 * - 不同模组可能会出现坐标轴交换/镜像
 * - 如果你看到坐标变化但“点不到按钮”，再按现象打开这些宏
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

bool dev_touch_init(void)
{
  if (s_inited)
  {
    return true;
  }

  if (GTP_Init_Panel() != 0)
  {
    return false;
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

  int tx = 0;
  int ty = 0;
  int touch_num = GTP_Execu(&tx, &ty);

  if (touch_num <= 0)
  {
    *pressed = false;
    *x = 0;
    *y = 0;
    return true;
  }

  uint16_t px = (tx < 0) ? 0u : (uint16_t)tx;
  uint16_t py = (ty < 0) ? 0u : (uint16_t)ty;

#if DEV_TOUCH_SWAP_XY
  uint16_t tmp = px;
  px = py;
  py = tmp;
#endif
#if DEV_TOUCH_INVERT_X
  if (dev_lcd_width() > 0u)
    px = (uint16_t)((uint16_t)dev_lcd_width() - 1u - px);
#endif
#if DEV_TOUCH_INVERT_Y
  if (dev_lcd_height() > 0u)
    py = (uint16_t)((uint16_t)dev_lcd_height() - 1u - py);
#endif

  /* 做一次保护，避免坐标越界影响 LVGL 命中测试 */
  if (px >= dev_lcd_width())
    px = (uint16_t)(dev_lcd_width() - 1u);
  if (py >= dev_lcd_height())
    py = (uint16_t)(dev_lcd_height() - 1u);

  *pressed = true;
  *x = px;
  *y = py;
  return true;
}

