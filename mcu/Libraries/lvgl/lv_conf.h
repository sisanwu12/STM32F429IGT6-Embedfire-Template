/**
 * @file lv_conf.h
 *
 * LVGL 配置文件（面向 STM32F429 + FreeRTOS + LTDC(RGB565) + SDRAM）
 *
 * 原则：
 * - 只覆盖“必须明确”的关键配置，其余交给 `src/lv_conf_internal.h` 默认值
 * - 先跑通“漂亮启动界面 + 中文”，后续再逐步开启更多 LVGL 功能
 */

#ifndef LV_CONF_H
#define LV_CONF_H

/*====================
 * COLOR SETTINGS
 *====================*/

/* 16: RGB565（与当前 LTDC 帧缓冲一致） */
#define LV_COLOR_DEPTH 16

/*=================
 * OPERATING SYSTEM
 *=================*/

/*
 * 当前工程在单独的 FreeRTOS 任务里调用 `lv_timer_handler()`
 * tick 在 SysTick 中断里通过 `lv_tick_inc(1)` 提供
 * 因此 LVGL 侧无需启用 OS 适配层（保持移植面最小）
 */
#define LV_USE_OS LV_OS_NONE

/*====================
 * LOG SETTINGS
 *====================*/

/* 关闭日志，减少体积与开销（需要时可改为 LV_LOG_LEVEL_WARN/INFO） */
#define LV_LOG_LEVEL LV_LOG_LEVEL_NONE

/*====================
 * MEMORY SETTINGS
 *====================*/

/*
 * LVGL 内存池放到外部 SDRAM（FMC Bank2: 0xD0000000）
 *
 * 约定：
 * - 帧缓冲：0xD0000000 起（800*480*2 ≈ 768KB）
 * - LVGL heap：0xD0100000 起（默认 512KB）
 *
 * 若你后续启用更大字体/图片缓存/双缓冲，可再调整地址与大小。
 */
#define LV_MEM_ADR 0xD0100000U
#define LV_MEM_SIZE (512U * 1024U)

/*==================
 * FONT SETTINGS
 *==================*/

/*
 * 中文字体：
 * - 使用 LVGL 自带的 Source Han Sans SC（常用 CJK radicals 子集）
 * - 注意：它不是完整汉字全集；如果你需要更全的字库，建议后续用字体转换工具生成定制字库
 */
#define LV_FONT_SOURCE_HAN_SANS_SC_16_CJK 1

/* 声明该字体，并设置为默认字体 */
#define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(lv_font_source_han_sans_sc_16_cjk)
#define LV_FONT_DEFAULT &lv_font_source_han_sans_sc_16_cjk

/* 字符编码：UTF-8（用于中文字符串常量） */
#define LV_TXT_ENC LV_TXT_ENC_UTF8

/*==================
 * THEMES
 *==================*/

/* 移植初期先关闭主题（后续若需要更“系统级”的统一外观再开启） */
#define LV_USE_THEME_DEFAULT 0
#define LV_USE_THEME_SIMPLE 0
#define LV_USE_THEME_MONO 0

/*==================
 * OTHERS
 *==================*/

/* 移植初期先关闭额外模块，避免引入更多源码 */
#define LV_USE_SYSMON 0
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0
#define LV_USE_TEST 0
#define LV_USE_OBSERVER 0
#define LV_USE_TRANSLATION 0

/*
 * 由于上游 `lv_init.c` 会无条件 `#include` 各类 libs/widget 的头文件，
 * 这里显式关闭我们暂时不需要的模块，避免后续误开启导致链接缺符号。
 *
 * 说明：
 * - 关闭并不等于不能用：你后续需要图片解码/字体渲染等功能时，再按需把对应目录
 *   从根目录 `lvgl-9.4.0/src/...` 拷贝进来即可。
 */
#define LV_USE_BMP 0
#define LV_USE_GIF 0
#define LV_USE_FFMPEG 0
#define LV_USE_FREETYPE 0
#define LV_USE_FS_DUMMY 0
#define LV_USE_TJPGD 0
#define LV_USE_LODEPNG 0
#define LV_USE_LIBPNG 0
#define LV_USE_LIBJPEG_TURBO 0
#define LV_USE_TINY_TTF 0
#define LV_USE_XML 0
#define LV_USE_SPAN 0

/* bin 解码器可能会用到的压缩算法（当前不用，先关掉） */
#define LV_USE_RLE 0
#define LV_USE_LZ4 0
#define LV_USE_LZ4_EXTERNAL 0
#define LV_USE_LZ4_INTERNAL 0

/*==================
 * DRAW SETTINGS
 *==================*/

/* Cortex-M4 无 NEON/Helium，明确禁用汇编加速 */
#define LV_USE_DRAW_SW_ASM LV_DRAW_SW_ASM_NONE

/* 不启用 DMA2D 移植（后续需要加速再启用并补齐 HAL 适配） */
#define LV_USE_DRAW_DMA2D 0

/*==================
 * WIDGETS
 *==================*/

/* 启动界面只依赖 label（其余控件后续按需开启） */
#define LV_USE_LABEL 1

#endif /*LV_CONF_H*/
