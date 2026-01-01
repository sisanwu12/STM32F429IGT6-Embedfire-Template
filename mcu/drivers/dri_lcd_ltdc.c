#include "dri_lcd_ltdc.h"

#include "boa_lcd_backlight.h"

static LTDC_HandleTypeDef hltdc;

static uint32_t s_fb_addr = 0;
static uint32_t s_w = 0;
static uint32_t s_h = 0;
static dri_lcd_fb_format_t s_fb_format = DRI_LCD_FB_RGB565;

HAL_StatusTypeDef dri_lcd_ltdc_init(const dri_lcd_ltdc_cfg_t *cfg)
{
  if (cfg == NULL || cfg->width == 0 || cfg->height == 0)
  {
    return HAL_ERROR;
  }

  s_fb_addr = cfg->framebuffer_addr;
  s_w = cfg->width;
  s_h = cfg->height;
  s_fb_format = cfg->fb_format;

  /*
   * LTDC 时序初始化：
   * - 关键：LCD_HSYNC/HBP/HFP/VSYNC/VBP/VFP 必须匹配面板
   * - 分辨率与时序由 devices/ 提供（通过 cfg 传入）
   */
  hltdc.Instance = LTDC;

  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  /*
   * DE（Data Enable）极性：
   * - 大多数 RGB TTL 屏是 DE 高有效
   * - 之前使用低有效更容易出现“显示区域不对/黑边”等现象
   *
   * 如果你后续发现显示仍然异常，可尝试切回 LTDC_DEPOLARITY_AL 对比现象差异。
   */
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AH;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

  hltdc.Init.HorizontalSync = (uint32_t)(cfg->timing.hsync - 1u);
  hltdc.Init.VerticalSync = (uint32_t)(cfg->timing.vsync - 1u);

  hltdc.Init.AccumulatedHBP =
      (uint32_t)(cfg->timing.hsync + cfg->timing.hbp - 1u);
  hltdc.Init.AccumulatedVBP =
      (uint32_t)(cfg->timing.vsync + cfg->timing.vbp - 1u);

  hltdc.Init.AccumulatedActiveW =
      (uint32_t)(cfg->timing.hsync + cfg->timing.hbp + cfg->width - 1u);
  hltdc.Init.AccumulatedActiveH =
      (uint32_t)(cfg->timing.vsync + cfg->timing.vbp + cfg->height - 1u);

  hltdc.Init.TotalWidth =
      (uint32_t)(cfg->timing.hsync + cfg->timing.hbp + cfg->width +
                 cfg->timing.hfp - 1u);
  hltdc.Init.TotalHeigh =
      (uint32_t)(cfg->timing.vsync + cfg->timing.vbp + cfg->height +
                 cfg->timing.vfp - 1u);

  hltdc.Init.Backcolor.Red = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Blue = 0;

  HAL_StatusTypeDef status = HAL_LTDC_Init(&hltdc);
  if (status != HAL_OK)
  {
    return status;
  }

  /*
   * 配置 Layer0 读取帧缓冲并输出到面板
   */
  LTDC_LayerCfgTypeDef layer_cfg = {0};
  layer_cfg.WindowX0 = 0;
  layer_cfg.WindowX1 = cfg->width;
  layer_cfg.WindowY0 = 0;
  layer_cfg.WindowY1 = cfg->height;

  if (cfg->fb_format == DRI_LCD_FB_RGB565)
  {
    layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  }
  else
  {
    layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  }

  layer_cfg.FBStartAdress = (uint32_t)cfg->framebuffer_addr;
  layer_cfg.Alpha = 255;
  layer_cfg.Alpha0 = 0;
  layer_cfg.Backcolor.Red = 0;
  layer_cfg.Backcolor.Green = 0;
  layer_cfg.Backcolor.Blue = 0;
  layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  layer_cfg.ImageWidth = cfg->width;
  layer_cfg.ImageHeight = cfg->height;

  status = HAL_LTDC_ConfigLayer(&hltdc, &layer_cfg, 0);
  if (status != HAL_OK)
  {
    return status;
  }

  boa_lcd_backlight_on();
  return HAL_OK;
}

void dri_lcd_fill_rgb565(uint16_t rgb565)
{
  if (s_fb_format != DRI_LCD_FB_RGB565)
  {
    (void)rgb565;
    return;
  }

  /*
   * 最简单的“单色填充”：
   * - 直接把帧缓冲每个像素写成同一个 RGB565 值
   *
   * 注意：
   * - 帧缓冲位于外部 SDRAM，地址为 LTDC_BUFF_ADDR
   * - 在 LTDC 已经启动读取之前填充，能更直观地看到效果
   */
  volatile uint16_t *fb = (volatile uint16_t *)(s_fb_addr);
  uint32_t pixel_count = (uint32_t)s_w * (uint32_t)s_h;

  for (uint32_t i = 0; i < pixel_count; i++)
  {
    fb[i] = rgb565;
  }
}

void *dri_lcd_framebuffer(void)
{
  return (void *)s_fb_addr;
}

LTDC_HandleTypeDef *dri_lcd_ltdc_handle(void)
{
  return &hltdc;
}
