#pragma once

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * 基于 LTDC 的 LCD 初始化与单色填充示例
 *
 * 使用方式（最小流程）：
 * 1) HAL_Init()
 * 2) SystemClock_Config()（必须：给 LTDC/SDRAM 提供正确时钟）
 * 3) dri_sdram_init()
 * 4) dri_lcd_ltdc_init()
 * 5) dri_lcd_fill_rgb565(...)
 */

HAL_StatusTypeDef dri_lcd_ltdc_init(void);

/* 仅示例：RGB565 单色填充 */
void dri_lcd_fill_rgb565(uint16_t rgb565);

/* 返回 LTDC 帧缓冲指针（用于 LVGL 等上层） */
void *dri_lcd_framebuffer(void);

/* 返回内部保存的 LTDC handle，便于调试/扩展 */
LTDC_HandleTypeDef *dri_lcd_ltdc_handle(void);

#ifdef __cplusplus
}
#endif
