/**
 * @file lv_sysmon_private.h
 *
 * 本工程的最小移植桩：
 * - LVGL core/lv_global.h 会 include 该头文件
 * - 本工程关闭 LV_USE_SYSMON/LV_USE_MEM_MONITOR，因此无需提供实现
 */

#ifndef LV_SYSMON_PRIVATE_H
#define LV_SYSMON_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../lv_conf_internal.h"

/* lv_sysmon_backend_data_t 仅在 LV_USE_MEM_MONITOR 时会用到 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SYSMON_PRIVATE_H*/

