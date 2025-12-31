#include "app_main.h"

#include "dri_lcd_ltdc.h"
#include "dri_sdram.h"
#include "dri_lcd_panel.h"
#include "ser_lvgl.h"

/*
 * app_init：
 * - 初始化与“产品功能相关”的外设组合（这里是 SDRAM + LCD）
 * - 不启动调度器，不阻塞
 */
void app_init(void)
{
  /* 1) 初始化 SDRAM（帧缓冲在外部 SDRAM） */
  if (dri_sdram_init() != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* 2) 初始化 LTDC 并点亮背光 */
  if (dri_lcd_ltdc_init() != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* 3) 上电默认填充一个纯色，确认显示链路正常 */
  dri_lcd_fill_rgb565(LCD_COLOR_BLUE_RGB565);
}

/*
 * app_start：
 * - 创建/启动 app 层与 service 层任务
 * - 由 core/main.c 在 vTaskStartScheduler() 前调用
 */
void app_start(void)
{
  /*
   * LVGL 任务启动（若当前工程尚未加入 LVGL 源码/头文件，该函数会退化为空实现）
   * 你后续把 LVGL 加进来后，这里无需再改。
   */
  ser_lvgl_start();
}

