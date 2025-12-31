#include "stm32f4xx_hal.h"

/*
 * 本文件提供 LTDC/SDRAM 的 MSP 初始化（GPIO + 时钟，按 drivers/dri_* 命名规范）
 *
 * 设计目标：
 * - 不修改 ST 的 HAL 库源码（mcu/Libraries/...）
 * - 让 HAL_LTDC_Init / HAL_SDRAM_Init 能够真正把外设引脚拉起来
 *
 * 引脚映射依据：
 * - doc/原理图/野火_F429挑战者_核心板_原理图_V2.1.pdf
 * - doc/原理图/野火_F429_F767_H743挑战者_底板_原理图_V2.0_2024.pdf
 */

/* ==========================
 * LTDC MSP
 * ========================== */
void HAL_LTDC_MspInit(LTDC_HandleTypeDef *hltdc)
{
  if (hltdc->Instance != LTDC)
  {
    return;
  }

  /* 1) 使能 LTDC 时钟 */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /* 2) 使能相关 GPIO 时钟 */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();

  /*
   * 3) 配置 LTDC 引脚（AF14）
   *
   * 原理图中的典型映射（RGB888 + HS/VS/DE/CLK）：
   * - R0  PH2   - R1  PH3   - R2  PH8   - R3  PB0
   * - R4  PA11  - R5  PA12  - R6  PB1   - R7  PG6
   * - G0  PE5   - G1  PE6   - G2  PH13  - G3  PG10
   * - G4  PH15  - G5  PI0   - G6  PC7   - G7  PI2
   * - B0  PE4   - B1  PG12  - B2  PD6   - B3  PG11
   * - B4  PI4   - B5  PA3   - B6  PB8   - B7  PB9
   * - VS  PI9   - HS  PI10  - CLK PG7   - DE  PF10
   *
   * 注意：
   * - 不同底板版本可能有差异；如果你发现部分颜色通道不对，请对照原理图修正。
   */
  GPIO_InitTypeDef gpio = {0};
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF14_LTDC;

  /* GPIOA: PA3(B5), PA11(R4), PA12(R5) */
  gpio.Pin = GPIO_PIN_3 | GPIO_PIN_11 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOA, &gpio);

  /* GPIOB: PB0(R3), PB1(R6), PB8(B6), PB9(B7) */
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9;
  HAL_GPIO_Init(GPIOB, &gpio);

  /* GPIOC: PC7(G6) */
  gpio.Pin = GPIO_PIN_7;
  HAL_GPIO_Init(GPIOC, &gpio);

  /* GPIOD: PD6(B2) */
  gpio.Pin = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &gpio);

  /* GPIOE: PE4(B0), PE5(G0), PE6(G1) */
  gpio.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOE, &gpio);

  /* GPIOF: PF10(DE) */
  gpio.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &gpio);

  /* GPIOG: PG6(R7), PG7(CLK), PG10(G3), PG11(B3), PG12(B1) */
  gpio.Pin =
      GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &gpio);

  /* GPIOH: PH2(R0), PH3(R1), PH8(R2), PH13(G2), PH15(G4) */
  gpio.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_8 | GPIO_PIN_13 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOH, &gpio);

  /* GPIOI: PI0(G5), PI2(G7), PI4(B4), PI9(VS), PI10(HS) */
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_9 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOI, &gpio);

  /* 4) 使能 LTDC 中断（可选：本示例不依赖中断，但打开也无妨） */
  HAL_NVIC_SetPriority(LTDC_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(LTDC_IRQn);
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef *hltdc)
{
  if (hltdc->Instance != LTDC)
  {
    return;
  }

  __HAL_RCC_LTDC_CLK_DISABLE();
  HAL_NVIC_DisableIRQ(LTDC_IRQn);
}

/* ==========================
 * SDRAM MSP（FMC）
 * ========================== */
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
  if (hsdram->Instance != FMC_SDRAM_DEVICE)
  {
    return;
  }

  /* 1) 使能 FMC 时钟 */
  __HAL_RCC_FMC_CLK_ENABLE();

  /* 2) 使能相关 GPIO 时钟 */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*
   * 3) 配置 FMC SDRAM 引脚（AF12）
   *
   * 这里按 STM32F429 常见 16bit SDRAM 连接方式配置一组“覆盖面较全”的引脚集合，
   * 与 doc/ 原理图中的 FMC 引脚匹配度较高：
   * - 地址线：PF0..PF5, PF11..PF15, PG0..PG2
   * - 数据线：PD14,PD15,PD0,PD1, PE7..PE15, PD8..PD10
   * - 控制线：PC0(WE), PF11(RAS), PG15(CAS), PG4(BA0), PG5(BA1),
   *          PH7(CKE), PG8(CLK), PG9?（若有）, PH6(SDNE0/CS), ...（视硬件而定）
   *
   * 注意：
   * - 不同板卡可能将 SDNE0/SDNE1 连接不同；本示例默认使用 Bank1（SDNE0）
   * - 若你的硬件使用 Bank2（SDNE1），需要改 dri_sdram_init() 里的 Bank 选择与地址
   */
  GPIO_InitTypeDef gpio = {0};
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF12_FMC;

  /* GPIOF: A0-A5, A6-A9, A10-A13, RAS */
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
             GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
             GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &gpio);

  /* GPIOG: A10-A12, BA0/BA1, CAS, CLK */
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 |
             GPIO_PIN_8 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &gpio);

  /* GPIOH: SDNE0/CS, CKE */
  gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  HAL_GPIO_Init(GPIOH, &gpio);

  /* GPIOC: WE */
  gpio.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOC, &gpio);

  /* GPIOD: D0-D3, D13-D15 */
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
             GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &gpio);

  /* GPIOE: D4-D12 */
  gpio.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
             GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &gpio);

  /*
   * GPIOE: NBL0/NBL1（字节选通/DQM）
   *
   * 在 16bit SDRAM 总线上，NBL0/NBL1 通常用于选择低/高字节，缺少它们时：
   * - 写入可能无效或数据异常
   * - LTDC 读取帧缓冲会出现“白屏/花屏/颜色不对”等现象
   *
   * 野火 F429 挑战者常见连接：
   * - FMC_NBL0 -> PE0
   * - FMC_NBL1 -> PE1
   */
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  HAL_GPIO_Init(GPIOE, &gpio);
}

void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram)
{
  if (hsdram->Instance != FMC_SDRAM_DEVICE)
  {
    return;
  }

  __HAL_RCC_FMC_CLK_DISABLE();
}
