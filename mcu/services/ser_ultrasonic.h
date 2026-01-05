#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * services/ 层：超声波测距服务
 *
 * - 独立任务周期采样距离（避免阻塞 LVGL 任务）
 * - 提供最新距离给 UI 显示
 */

void ser_ultrasonic_start(void);

/* 读取最新一次有效距离（mm）；无有效数据返回 false */
bool ser_ultrasonic_get_latest_mm(uint32_t *mm);

#ifdef __cplusplus
} /*extern "C"*/
#endif
