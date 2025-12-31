/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

  /* Private includes
   * ----------------------------------------------------------*/
  /* USER CODE BEGIN Includes */

  /* USER CODE END Includes */

  /* Exported types
   * ------------------------------------------------------------*/
  /* USER CODE BEGIN ET */

  /* USER CODE END ET */

  /* Exported constants
   * --------------------------------------------------------*/
  /* USER CODE BEGIN EC */

  /* USER CODE END EC */

  /* Exported macro
   * ------------------------------------------------------------*/
  /* USER CODE BEGIN EM */

  /* USER CODE END EM */

  /* Exported functions prototypes
   * ---------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/*
 * LTDC 帧缓冲地址
 *
 * 重要：野火 F429 挑战者这类板卡为了避免与 LTDC 引脚冲突，SDRAM 通常挂在 FMC Bank2：
 * - SDNE1/SDCKE1 使用 PH6/PH7
 * - 对应 SDRAM 映射基地址为 0xD0000000
 *
 * 如果你的板卡确实把 SDRAM 接在 Bank1（PH3/PH2），才使用 0xC0000000。
 */
#define LTDC_BUFF_ADDR 0xD0000000
#define PIXELS_W 800
#define PIXELS_H 480
#define PIXELS_DIR 0

  /* USER CODE BEGIN Private defines */

  /* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
