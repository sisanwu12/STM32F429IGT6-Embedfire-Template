#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * touch/：Goodix GT9xx/GT615 触控库的硬件适配层（HAL）
 *
 * 该目录内容来自你在根目录 `touch/` 中验证可用的工程，但这里做了“最小移植”：
 * - 使用本项目的 HAL + `dri_i2c2` 驱动
 * - 使用 board/ 中的 `boa_touch` 做 RST/INT 复位时序
 *
 * 目标：稳定读到 0x814E 的触摸点数据，并供 devices/dev_touch 封装给 LVGL。
 */

/* GT9xx 常见 7-bit 地址（由上电复位时 INT 电平决定） */
#define BSP_TOUCH_ADDR_7BIT_5D 0x5Du
#define BSP_TOUCH_ADDR_7BIT_14 0x14u

/* 初始化 I2C + 执行一次芯片复位（会自动选择可用 I2C 地址） */
void I2C_Touch_Init(void);

/* 仅执行一次复位时序（不做 I2C init / 地址探测） */
void I2C_ResetChip(void);

/* 返回当前探测到的 7-bit I2C 地址（默认 0x5D） */
uint16_t bsp_touch_addr_7bit(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

