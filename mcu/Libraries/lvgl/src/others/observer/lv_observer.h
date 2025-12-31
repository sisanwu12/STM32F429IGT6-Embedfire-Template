/**
 * @file lv_observer.h
 *
 * 本工程的最小移植桩：
 * - LVGL label 模块会 include 该头文件
 * - 本工程关闭 LV_USE_OBSERVER，因此无需提供实现
 */

#ifndef LV_OBSERVER_H
#define LV_OBSERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../lv_conf_internal.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBSERVER_H*/

