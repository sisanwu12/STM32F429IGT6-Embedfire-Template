#pragma once

#include "stm32f4xx_hal.h"

/*
 * 外部 SDRAM 初始化（FMC SDRAM）
 *
 * 作用：
 * - 为 LTDC 帧缓冲提供足够大的显存（工程默认 LTDC_BUFF_ADDR = 0xC0000000）
 *
 * 注意：
 * - 该工程目前没有 CubeMX 生成的 FMC/SDRAM 初始化代码，本文件补齐最小可用实现。
 * - SDRAM 芯片与时序需要与你的硬件匹配；根据 doc/ 原理图，核心板常见为 W9825G6KH-6（16bit）
 */

#ifdef __cplusplus
extern "C"
{
#endif

HAL_StatusTypeDef dri_sdram_init(void);

/* 返回内部保存的 SDRAM handle，便于调试/扩展 */
SDRAM_HandleTypeDef *dri_sdram_handle(void);

#ifdef __cplusplus
}
#endif
