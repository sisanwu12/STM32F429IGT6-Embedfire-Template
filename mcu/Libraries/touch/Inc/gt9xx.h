#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * touch/：Goodix GT9xx/GT615 驱动（最小移植版）
 *
 * 说明：
 * - 仅保留本项目当前用到的两个接口：
 *   - `GTP_Init_Panel()`：初始化（I2C + 复位 + 基本连通性校验）
 *   - `GTP_Execu()`：轮询读取单点触摸坐标
 * - 触摸点读取协议来自你验证可用的 touch/Src/gt9xx.c：
 *   - 从 0x814E 读取状态 + 点数据
 *   - 读完写 0 清状态
 */

int32_t GTP_Init_Panel(void);

/*
 * 读取单点触摸：
 * - x/y 输出像素坐标（控制器内部通常已按屏分辨率输出）
 * - 返回值：当前触摸点数（0 表示无触摸）
 */
int GTP_Execu(int *x, int *y);

#ifdef __cplusplus
} /*extern "C"*/
#endif

