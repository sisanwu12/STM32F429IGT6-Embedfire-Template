#include "dri_lcd_ltdc.h"

#include "dri_lcd_panel.h"
#include "main.h"

static LTDC_HandleTypeDef hltdc;

static void LCD_Backlight_On(void)
{
  /*
   * 根据 doc/ 原理图（核心板/底板）：
   * - LCD 背光控制脚为 PD7（标注 LCD_BL）
   * - 这里先用 GPIO 输出高电平打开背光（若你的硬件是 PWM 调光，可改为 TIM PWM）
   */
  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_InitTypeDef gpio = {0};
  gpio.Pin = GPIO_PIN_7;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &gpio);

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET);
}

HAL_StatusTypeDef dri_lcd_ltdc_init(void)
{
  /*
   * LTDC 时序初始化：
   * - 关键：LCD_HSYNC/HBP/HFP/VSYNC/VBP/VFP 必须匹配面板
   * - 面板分辨率来自 main.h：PIXELS_W / PIXELS_H
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

  hltdc.Init.HorizontalSync = (uint32_t)(LCD_HSYNC - 1u);
  hltdc.Init.VerticalSync = (uint32_t)(LCD_VSYNC - 1u);

  hltdc.Init.AccumulatedHBP = (uint32_t)(LCD_HSYNC + LCD_HBP - 1u);
  hltdc.Init.AccumulatedVBP = (uint32_t)(LCD_VSYNC + LCD_VBP - 1u);

  hltdc.Init.AccumulatedActiveW =
      (uint32_t)(LCD_HSYNC + LCD_HBP + PIXELS_W - 1u);
  hltdc.Init.AccumulatedActiveH =
      (uint32_t)(LCD_VSYNC + LCD_VBP + PIXELS_H - 1u);

  hltdc.Init.TotalWidth =
      (uint32_t)(LCD_HSYNC + LCD_HBP + PIXELS_W + LCD_HFP - 1u);
  hltdc.Init.TotalHeigh =
      (uint32_t)(LCD_VSYNC + LCD_VBP + PIXELS_H + LCD_VFP - 1u);

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
  layer_cfg.WindowX1 = PIXELS_W;
  layer_cfg.WindowY0 = 0;
  layer_cfg.WindowY1 = PIXELS_H;

#if LCD_FB_FORMAT_RGB565
  layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
#else
  layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
#endif

  layer_cfg.FBStartAdress = (uint32_t)LTDC_BUFF_ADDR;
  layer_cfg.Alpha = 255;
  layer_cfg.Alpha0 = 0;
  layer_cfg.Backcolor.Red = 0;
  layer_cfg.Backcolor.Green = 0;
  layer_cfg.Backcolor.Blue = 0;
  layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  layer_cfg.ImageWidth = PIXELS_W;
  layer_cfg.ImageHeight = PIXELS_H;

  status = HAL_LTDC_ConfigLayer(&hltdc, &layer_cfg, 0);
  if (status != HAL_OK)
  {
    return status;
  }

  LCD_Backlight_On();
  return HAL_OK;
}

void dri_lcd_fill_rgb565(uint16_t rgb565)
{
#if !LCD_FB_FORMAT_RGB565
  (void)rgb565;
  return;
#else
  /*
   * 最简单的“单色填充”：
   * - 直接把帧缓冲每个像素写成同一个 RGB565 值
   *
   * 注意：
   * - 帧缓冲位于外部 SDRAM，地址为 LTDC_BUFF_ADDR
   * - 在 LTDC 已经启动读取之前填充，能更直观地看到效果
   */
  volatile uint16_t *fb = (volatile uint16_t *)(LTDC_BUFF_ADDR);
  uint32_t pixel_count = (uint32_t)PIXELS_W * (uint32_t)PIXELS_H;

  for (uint32_t i = 0; i < pixel_count; i++)
  {
    fb[i] = rgb565;
  }
#endif
}

void *dri_lcd_framebuffer(void)
{
  return (void *)LTDC_BUFF_ADDR;
}

LTDC_HandleTypeDef *dri_lcd_ltdc_handle(void)
{
  return &hltdc;
}
