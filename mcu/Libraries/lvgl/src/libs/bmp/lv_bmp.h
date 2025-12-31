/**
 * @file lv_bmp.h
 *
 * 本工程裁剪 LVGL 时保留的“占位头文件”。
 *
 * 目的：
 * - 上游 `lv_init.c` 会无条件 `#include "libs/bmp/lv_bmp.h"`
 * - 但我们当前在 `lv_conf.h` 中关闭了 `LV_USE_BMP`
 *
 * 当你后续需要 BMP 解码时：
 * - 从根目录 `lvgl-9.4.0/src/libs/bmp/` 按需拷贝实现文件到工程内即可。
 */

#ifndef LV_BMP_H
#define LV_BMP_H

#ifdef __cplusplus
extern "C" {
#endif

void lv_bmp_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_BMP_H*/

