/**
 * @file lv_ffmpeg.h
 *
 * 占位头文件：用于通过 `lv_init.c` 的无条件 include。
 * 当前工程在 `lv_conf.h` 中关闭了 `LV_USE_FFMPEG`。
 */

#ifndef LV_FFMPEG_H
#define LV_FFMPEG_H

#ifdef __cplusplus
extern "C" {
#endif

void lv_ffmpeg_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_FFMPEG_H*/

