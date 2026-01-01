#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * board/ 层：LCD 背光控制
 *
 * 说明：
 * - 背光控制脚属于板级连线信息（PinMap），因此放在 board/。
 * - 目前使用 GPIO 推挽输出开关背光；若你后续需要 PWM 调光，可在此扩展。
 */

void boa_lcd_backlight_init(void);
void boa_lcd_backlight_set(bool on);

static inline void boa_lcd_backlight_on(void)
{
  boa_lcd_backlight_set(true);
}

#ifdef __cplusplus
} /*extern "C"*/
#endif

