#include "app_main.h"

#include "dev_lcd.h"
#include "dev_lcd_panel.h"
#include "dev_sdram.h"
#include "ser_lvgl.h"
#include "ser_ultrasonic.h"

/*
 * app_init：
 * - 初始化
 * - 不启动调度器
 */
void app_init(void)
{
  /* 1) 初始化 SDRAM（帧缓冲在外部 SDRAM） */
  if (dev_sdram_init() != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* 2) 初始化 LTDC 并点亮背光 */
  if (dev_lcd_init() != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* 3) 上电默认填充一个纯色，确认显示链路正常 */
  dev_lcd_fill_rgb565(LCD_COLOR_BLUE_RGB565);
}

/*
 * app_start：
 * - 创建/启动 app 层与 service 层任务
 * - 由 core/main.c 在 vTaskStartScheduler() 前调用
 */
void app_start(void)
{
  /* 超声波测距服务：独立任务采样，供 UI 显示 */
  ser_ultrasonic_start();

  /*
   * LVGL 任务启动
   */
  ser_lvgl_start();
}
