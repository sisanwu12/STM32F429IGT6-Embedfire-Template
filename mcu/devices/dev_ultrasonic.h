#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * devices/ 层：超声波测距模块（CS100A/HC-SR04 类）
 *
 * 对外提供“距离(mm)”读取，内部通过 TRIG/ECHO 脉宽计算。
 */

bool dev_ultrasonic_init(void);

/* 阻塞测一次距离；成功返回 true，mm 输出距离（单位：毫米） */
bool dev_ultrasonic_measure_mm(uint32_t *mm);

/*
 * ISR 回调：ECHO 上下沿（EXTI）到来时调用。
 * - 由 core/ 中断入口转发（见 mcu/core/stm32f4xx_it.c）
 * - 仅用于捕获脉宽，不调用 FreeRTOS API
 */
void dev_ultrasonic_echo_edge_isr(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif
