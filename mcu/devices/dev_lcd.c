#include "dev_lcd.h"

#include "dev_lcd_panel.h"

#include "dri_lcd_ltdc.h"

/*
 * 当前工程的默认 LCD 配置：
 * - 分辨率：800x480
 * - 帧缓冲：外部 SDRAM Bank2 起始（0xD0000000）
 * - 像素格式：RGB565
 *
 * 如果你更换屏幕/分辨率/帧缓冲位置，请在这里与 dev_lcd_panel.h 中同步修改。
 */
#define DEV_LCD_DEFAULT_FB_ADDR 0xD0000000u
#define DEV_LCD_DEFAULT_W 800u
#define DEV_LCD_DEFAULT_H 480u

static const dri_lcd_ltdc_cfg_t s_cfg = {
    .width = DEV_LCD_DEFAULT_W,
    .height = DEV_LCD_DEFAULT_H,
    .framebuffer_addr = DEV_LCD_DEFAULT_FB_ADDR,
#if LCD_FB_FORMAT_RGB565
    .fb_format = DRI_LCD_FB_RGB565,
#else
    .fb_format = DRI_LCD_FB_ARGB8888,
#endif
    .timing =
        {
            .hsync = LCD_HSYNC,
            .hbp = LCD_HBP,
            .hfp = LCD_HFP,
            .vsync = LCD_VSYNC,
            .vbp = LCD_VBP,
            .vfp = LCD_VFP,
        },
};

HAL_StatusTypeDef dev_lcd_init(void)
{
  return dri_lcd_ltdc_init(&s_cfg);
}

void dev_lcd_fill_rgb565(uint16_t rgb565)
{
  dri_lcd_fill_rgb565(rgb565);
}

void *dev_lcd_framebuffer(void)
{
  return dri_lcd_framebuffer();
}

uint16_t dev_lcd_width(void)
{
  return (uint16_t)s_cfg.width;
}

uint16_t dev_lcd_height(void)
{
  return (uint16_t)s_cfg.height;
}

