/**
 * @file lvgl.h
 *
 * 上游源码在 `mcu/Libraries/lvgl/src/` 下有时会用相对路径 `#include "../lvgl.h"`。
 * 本工程的入口头文件位于 `mcu/Libraries/lvgl/lvgl.h`，因此这里提供一个薄适配层：
 * - 让上游相对 include 可用
 * - 避免引入更多上游顶层文件结构
 */

#ifndef LVGL_SRC_LVGL_H
#define LVGL_SRC_LVGL_H

#include "../lvgl.h"

#endif /*LVGL_SRC_LVGL_H*/
