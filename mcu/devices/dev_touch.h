#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * devices/ 层：电容触摸屏（外部器件）抽象接口
 *
 * 当前实现：GT911（常见 7 寸 800x480 电容屏）
 * - 若你的触控 IC 不是 GT911，文件仍可编译，但触摸读数可能不正确
 */

bool dev_touch_init(void);

/*
 * 读取单点触摸（本工程用于 LVGL 指针输入）
 * - pressed: true 表示按下
 * - x/y: 触摸点坐标（像素坐标系，原点左上角）
 */
bool dev_touch_read(bool *pressed, uint16_t *x, uint16_t *y);

#ifdef __cplusplus
} /*extern "C"*/
#endif

