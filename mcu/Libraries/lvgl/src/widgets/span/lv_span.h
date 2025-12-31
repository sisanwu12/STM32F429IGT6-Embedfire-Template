/**
 * @file lv_span.h
 *
 * 占位头文件：用于通过 `lv_init.c` 的无条件 include。
 * 当前工程在 `lv_conf.h` 中关闭了 `LV_USE_SPAN`。
 */

#ifndef LV_SPAN_H
#define LV_SPAN_H

#ifdef __cplusplus
extern "C" {
#endif

void lv_span_stack_init(void);
void lv_span_stack_deinit(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SPAN_H*/

