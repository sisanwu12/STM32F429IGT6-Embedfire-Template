#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * LVGL service（服务层）
 *
 * 目标：
 * - 在不引入 LVGL 源码的前提下先把“工程结构/接口”搭好，便于你下一步移植
 * - 当工程中存在 lvgl.h（以及相关配置 lv_conf.h）时，自动启用 LVGL 初始化与任务
 *
 * 依赖方向：
 * - app -> services(ser_lvgl) -> drivers(dri_lcd_*)
 */

/* 创建 LVGL 任务（若 LVGL 未集成则为空实现） */
void ser_lvgl_start(void);

/* 给 SysTick/RTOS tick 调用：每 ms 调一次即可（若 LVGL 未集成则为空实现） */
void ser_lvgl_tick_inc_isr(uint32_t ms);

#ifdef __cplusplus
}
#endif

