#pragma once

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * drivers/ 层：微秒级时间基准（DWT CYCCNT）
 *
 * 用途：
 * - 生成 10~50us 量级 TRIG 脉冲
 * - 采集 ECHO 脉宽（超声波测距）
 *
 * 注意：
 * - DWT CYCCNT 是 32-bit，会溢出；本驱动只保证“短时间差”的正确性
 */

HAL_StatusTypeDef dri_time_us_init(void);

/* 返回当前 cycle counter（非 us），用于做短时间差计算 */
uint32_t dri_time_cycles_now(void);

/* 从 start_cycles 到现在经过的微秒数（适合 <~1s 的短时差） */
uint32_t dri_time_cycles_elapsed_us(uint32_t start_cycles);

/* 忙等延时（微秒） */
void dri_time_delay_us(uint32_t us);

#ifdef __cplusplus
} /*extern "C"*/
#endif

