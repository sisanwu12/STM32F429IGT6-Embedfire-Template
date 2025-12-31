/**
 * @file lv_freetype.h
 *
 * 占位头文件：用于通过 `lv_init.c` 的无条件 include。
 * 当前工程在 `lv_conf.h` 中关闭了 `LV_USE_FREETYPE`。
 */

#ifndef LV_FREETYPE_H
#define LV_FREETYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void lv_freetype_init(uint32_t max_glyph_cnt);
void lv_freetype_uninit(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_FREETYPE_H*/

