/**
 * @file lvgl.h
 *
 * 本工程内置的“精简版 LVGL 入口头文件”（适配 LVGL v9.4.0）
 *
 * 目标：
 * - 只暴露当前工程启动界面会用到的 API（display + tick + timer + obj + label + style）
 * - 避免引入上游 `lvgl.h` 中大量模块导致你不得不搬运更多源码
 *
 * 说明：
 * - 后续你需要更多控件时，再按需：
 *   1) 从根目录 `lvgl-9.4.0/src/widgets/<xxx>` 拷贝到 `mcu/Libraries/lvgl/src/widgets/<xxx>`
 *   2) 在此文件补上对应 `#include`
 */

#ifndef LVGL_H
#define LVGL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lv_version.h"

/*
 * 上游 `lvgl.h` 会提供该宏，部分内置字体源文件会用它做兼容判断。
 * 注意：该宏不是“>=某版本”的判断，而是“同一主版本下 >= 指定次/补丁版本”。
 */
#ifndef LV_VERSION_CHECK
#define LV_VERSION_CHECK(x, y, z)                                                    \
  ((x) == LVGL_VERSION_MAJOR &&                                                      \
   ((y) < LVGL_VERSION_MINOR || ((y) == LVGL_VERSION_MINOR && (z) <= LVGL_VERSION_PATCH)))
#endif

#include "src/lv_init.h"
#include "src/tick/lv_tick.h"
#include "src/misc/lv_timer.h"
#include "src/misc/lv_anim.h"

#include "src/misc/lv_style.h"
#include "src/misc/lv_color.h"

#include "src/display/lv_display.h"

#include "src/indev/lv_indev.h"

#include "src/core/lv_obj.h"
#include "src/core/lv_obj_pos.h"

#include "src/widgets/label/lv_label.h"

#include "src/font/lv_font.h"
#include "src/font/lv_font_fmt_txt.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVGL_H*/
