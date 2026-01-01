#pragma once

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * devices/ 层：LCD 屏幕（外部器件）对外接口
 *
 * 设计：
 * - app/ 与 services/ 只依赖 devices/ 提供的接口
 * - devices/ 内部组合 drivers/（LTDC、SDRAM 等）完成初始化与渲染
 */

HAL_StatusTypeDef dev_lcd_init(void);

void dev_lcd_fill_rgb565(uint16_t rgb565);
void *dev_lcd_framebuffer(void);

uint16_t dev_lcd_width(void);
uint16_t dev_lcd_height(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

