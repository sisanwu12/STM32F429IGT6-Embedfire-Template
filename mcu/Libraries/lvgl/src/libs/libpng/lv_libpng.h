/**
 * @file lv_libpng.h
 *
 * 占位头文件：用于通过 `lv_init.c` 的无条件 include。
 * 当前工程在 `lv_conf.h` 中关闭了 `LV_USE_LIBPNG`。
 */

#ifndef LV_LIBPNG_H
#define LV_LIBPNG_H

#ifdef __cplusplus
extern "C" {
#endif

void lv_libpng_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LIBPNG_H*/

