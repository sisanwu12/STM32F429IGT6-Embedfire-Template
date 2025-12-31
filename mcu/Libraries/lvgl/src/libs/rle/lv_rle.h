/**
 * @file lv_rle.h
 *
 * 占位头文件：用于通过 `lv_bin_decoder.c` 的无条件 include。
 * 当前工程在 `lv_conf.h` 中关闭了 `LV_USE_RLE`。
 *
 * 需要 RLE 支持时，请把上游 `lvgl-9.4.0/src/libs/rle/` 目录拷贝进来。
 */

#ifndef LV_RLE_H
#define LV_RLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t lv_rle_decompress(const uint8_t * input, uint32_t input_len,
                           uint8_t * output, uint32_t output_len,
                           uint32_t pixel_byte);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_RLE_H*/

