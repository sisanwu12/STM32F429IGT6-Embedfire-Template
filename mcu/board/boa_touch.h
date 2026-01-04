#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * board/ 层：电容触摸（CTP）引脚装配
 *
 * 来自原理图提取线索（挑战者底板/液晶模块）：
 * - I2C1_SCL: PB6
 * - I2C1_SDA: PB7
 * - CTP_INT : PD13
 * - CTP_RST : PI8（注意：PI10 已用于 LTDC_HS，本工程默认用 PI8 作为 RST）
 *
 * 如果你的硬件版本不同，请只改这里的 Pin 定义即可。
 */

void boa_touch_gpio_init(void);
void boa_touch_reset_pulse(void);
void boa_touch_rst_set(bool high);

/* INT 线通常为“低有效/上升沿触发”，这里提供读取/配置基础能力 */
bool boa_touch_int_is_active(void);
bool boa_touch_int_level_high(void);

/* 供设备层做 reset/addr 选择时使用（某些控制器会在 reset 阶段复用 INT 脚） */
void boa_touch_int_set_output(bool output);
void boa_touch_int_write(bool high);

/*
 * GT9xx/GT615 常用复位序列（来自已验证可用的官方示例）：
 * - 复位阶段 INT 作为输出用于选择 I2C 地址（INT=0 常对应 0x5D）
 * - RST 低/高并保持足够延时
 * - 释放 INT 为输入（浮空）
 */
void boa_touch_reset_for_gt9xx(bool int_high);

#ifdef __cplusplus
} /*extern "C"*/
#endif
