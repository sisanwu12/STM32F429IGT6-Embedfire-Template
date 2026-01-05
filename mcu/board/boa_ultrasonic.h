#pragma once

#include "stm32f4xx_hal.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * board/ 层：超声波测距模块引脚装配（CS100A/HC-SR04 类：TRIG/ECHO）
 *
 * 模块引脚（参考 doc/原理图/野火小智超声波测距模块原理图.pdf）：
 * - TRIG：输入一个 >10us 的高电平脉冲开始测距
 * - ECHO：输出高电平脉宽 = 超声往返时间
 *
 * 你需要根据自己接线修改下面的默认引脚（只改这里即可）。
 */

#ifndef BOA_US_TRIG_PORT
#define BOA_US_TRIG_PORT GPIOC
#endif
#ifndef BOA_US_TRIG_PIN
#define BOA_US_TRIG_PIN GPIO_PIN_1
#endif

#ifndef BOA_US_ECHO_PORT
#define BOA_US_ECHO_PORT GPIOC
#endif
#ifndef BOA_US_ECHO_PIN
#define BOA_US_ECHO_PIN GPIO_PIN_2
#endif

/*
 * ECHO 输入上下拉配置：
 * - 默认不使能内部上下拉（GPIO_NOPULL）
 * - 若你怀疑 ECHO 悬空/开漏，可尝试改为 GPIO_PULLUP 或 GPIO_PULLDOWN
 */
#ifndef BOA_US_ECHO_PULL
#define BOA_US_ECHO_PULL GPIO_NOPULL
#endif

void boa_ultrasonic_gpio_init(void);
void boa_ultrasonic_trig_write(bool high);
bool boa_ultrasonic_echo_read(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif
