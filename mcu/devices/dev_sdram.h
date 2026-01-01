#pragma once

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * devices/ 层：外部 SDRAM（外部器件）对外接口
 *
 * 说明：
 * - SDRAM 芯片属于外部器件，因此 app/ 不直接调用 drivers/ 的 FMC 初始化。
 * - 具体 FMC/SDRAM 的底层实现仍在 drivers/ 中（dri_sdram.*）。
 */

HAL_StatusTypeDef dev_sdram_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

