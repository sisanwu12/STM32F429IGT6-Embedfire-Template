#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @file dev_gt9xx.h
   * @brief devices 层：Goodix GT9xx/GT615 触摸芯片驱动（最小轮询版）
   * @author ssw12
   *
   * 说明：
   * - 依赖 drivers 层的 `dri_touch_gt9xx` 提供 I2C/复位/寄存器读写能力
   * - 当前仅实现单点触摸轮询读取（供 dev_touch / ser_lvgl / LVGL indev 使用）
   */

  /* 初始化并做一次最小连通性校验（读 0x8140） */
  bool dev_gt9xx_init(void);

  /*
   * 读取单点触摸：
   * - x/y 输出像素坐标（控制器内部通常已按屏分辨率输出）
   * - 返回值：当前触摸点数（0 表示无触摸）
   */
  int dev_gt9xx_read(int *x, int *y);

#ifdef __cplusplus
} /*extern "C"*/
#endif
