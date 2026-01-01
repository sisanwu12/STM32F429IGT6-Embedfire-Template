#pragma once

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * drivers/ 层：LTDC（片上外设）驱动
 *
 * 说明：
 * - drivers/ 只封装片上外设能力（LTDC），不硬编码外部面板参数
 * - 外部面板的分辨率/时序等由 devices/ 负责提供并传入配置
 */

typedef enum
{
  DRI_LCD_FB_RGB565 = 0,
  DRI_LCD_FB_ARGB8888 = 1,
} dri_lcd_fb_format_t;

typedef struct
{
  uint16_t hsync;
  uint16_t hbp;
  uint16_t hfp;
  uint16_t vsync;
  uint16_t vbp;
  uint16_t vfp;
} dri_lcd_ltdc_timing_t;

typedef struct
{
  uint32_t width;
  uint32_t height;
  uint32_t framebuffer_addr;
  dri_lcd_fb_format_t fb_format;
  dri_lcd_ltdc_timing_t timing;
} dri_lcd_ltdc_cfg_t;

HAL_StatusTypeDef dri_lcd_ltdc_init(const dri_lcd_ltdc_cfg_t *cfg);

/* 仅示例：RGB565 单色填充 */
void dri_lcd_fill_rgb565(uint16_t rgb565);

/* 返回 LTDC 帧缓冲指针（用于 LVGL 等上层） */
void *dri_lcd_framebuffer(void);

/* 返回内部保存的 LTDC handle，便于调试/扩展 */
LTDC_HandleTypeDef *dri_lcd_ltdc_handle(void);

#ifdef __cplusplus
}
#endif
