/* 引用文件 */
#include "FreeRTOS.h"
#include "app_main.h"
#include "stm32f4xx_hal.h"
#include "task.h"

/*
 * 系统时钟配置（180MHz，HSE=25MHz）
 *
 * 重要提示：
 * - LTDC 的像素时钟来自 PLLSAI；如果屏幕不亮/花屏，时序与 PCLK 多半需要按面板手册调整
 */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef rcc_osc = {0};
  RCC_ClkInitTypeDef rcc_clk = {0};
  RCC_PeriphCLKInitTypeDef rcc_periph = {0};

  /* 使能电源控制时钟，并设置电压调节等级 */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* 1) 配置主 PLL，得到 SYSCLK=180MHz */
  rcc_osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  rcc_osc.HSEState = RCC_HSE_ON;
  rcc_osc.PLL.PLLState = RCC_PLL_ON;
  rcc_osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  rcc_osc.PLL.PLLM = 25;
  rcc_osc.PLL.PLLN = 360;
  rcc_osc.PLL.PLLP = RCC_PLLP_DIV2;
  rcc_osc.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&rcc_osc) != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* 180MHz 需要开启 OverDrive（F429 常见配置） */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    while (1)
    {
    }
  }

  /* 2) 配置 AHB/APB 分频 */
  rcc_clk.ClockType =
      RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
      RCC_CLOCKTYPE_PCLK2;
  rcc_clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  rcc_clk.AHBCLKDivider = RCC_SYSCLK_DIV1;  /* HCLK = 180MHz */
  rcc_clk.APB1CLKDivider = RCC_HCLK_DIV4;   /* PCLK1 = 45MHz */
  rcc_clk.APB2CLKDivider = RCC_HCLK_DIV2;   /* PCLK2 = 90MHz */
  if (HAL_RCC_ClockConfig(&rcc_clk, FLASH_LATENCY_5) != HAL_OK)
  {
    while (1)
    {
    }
  }

  /*
   * 3) 配置 LTDC 外设时钟（PLLSAI）
   *
   * 说明：
   * - 这里给出一组“能跑起来、便于你后续微调”的参数
   * - 真正需要的像素时钟 PCLK 由你的面板总时序决定：
   *     PCLK ≈ TotalWidth * TotalHeight * FPS
   *
   * 经验上：
   * - 800x480@60Hz：PCLK 常在 30~33MHz 左右
   * - 1024x600@60Hz：PCLK 常在 45~55MHz 左右
   */
  rcc_periph.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  /*
   * LTDC 像素时钟（PCLK）
   *
   * 本工程当前面板时序（见 mcu/devices/dev_lcd_panel.h）：
   * - TotalW = 1057, TotalH = 526, FPS = 60
   * - PCLK ≈ 1057 * 526 * 60 ≈ 33.36 MHz
   *
   * LTDC 时钟计算（HSE=25MHz，PLLM=25 => 1MHz 输入 PLLSAI）：
   *   LTDCclk = (1MHz * PLLSAIN) / PLLSAIR / PLLSAIDivR
   *
   * 选择：PLLSAIN=267, PLLSAIR=2, DIVR=4 => 33.375MHz
   */
  rcc_periph.PLLSAI.PLLSAIN = 267;
  rcc_periph.PLLSAI.PLLSAIR = 2;
  rcc_periph.PLLSAIDivR = RCC_PLLSAIDIVR_4;
  if (HAL_RCCEx_PeriphCLKConfig(&rcc_periph) != HAL_OK)
  {
    while (1)
    {
    }
  }
}

int main(void)
{
  HAL_Init();

  /* 配置系统时钟（必须在外设初始化之前完成） */
  SystemClock_Config();

  /* app 初始化与任务创建（符合 README.md 分层结构） */
  app_init();
  app_start();

  vTaskStartScheduler();
}
