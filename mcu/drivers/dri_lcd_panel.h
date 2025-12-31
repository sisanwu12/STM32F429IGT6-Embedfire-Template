#pragma once

#include <stdint.h>

/*
 * LCD 面板参数（需要与你的实际屏幕/模块匹配）
 *
 * 说明：
 * - 该工程的原理图显示：屏幕通过 STM32F429 的 LTDC（RGB888）输出（可能经
 * RGB->LVDS 转换）
 * - 你确认屏幕分辨率为 800x480（WVGA），刷新率目标 60Hz
 *
 * 因此：这里把“分辨率”和“时序”分开定义，便于你根据实际面板修改。
 *   - 分辨率：来自 mcu/core/main.h 的 PIXELS_W / PIXELS_H
 *   - 时序：本文件的 LCD_xxx 宏（默认给出一组常见的 800x480@60Hz 参考值）
 *
 * 如果你不确定屏幕时序，请优先查你面板数据手册（Timing / DE mode / Sync
 * mode）。
 */

/* ==========================
 * LTDC 像素格式选择
 * ========================== */
/*
 * 为了简单与节省显存，示例默认使用 RGB565 帧缓冲：
 * - 每像素 2 字节（16bit）
 * - 适合单色填充演示
 *
 * 注意：即便面板物理接口是 RGB888，LTDC 也可以从内存读 RGB565，再输出到 RGB888
 * 引脚。
 */
#define LCD_FB_FORMAT_RGB565 1

/* 帧缓冲每像素字节数 */
#if LCD_FB_FORMAT_RGB565
#define LCD_FB_BYTES_PER_PIXEL 2u
#else
#define LCD_FB_BYTES_PER_PIXEL 4u /* ARGB8888 */
#endif

/* ==========================
 * LTDC 时序参数（重点：常需要改）
 * ========================== */
/*
 * LTDC 计算规则（HAL/寄存器语义）：
 * - HSYNC = LCD_HSYNC - 1
 * - VSYNC = LCD_VSYNC - 1
 * - AccumulatedHBP = HSYNC + HBP
 * - AccumulatedVBP = VSYNC + VBP
 * - AccumulatedActiveW = AccumulatedHBP + ActiveW
 * - AccumulatedActiveH = AccumulatedVBP + ActiveH
 * - TotalW = AccumulatedActiveW + HFP
 * - TotalH = AccumulatedActiveH + VFP
 *
 * 当前工程已验证的一组常见 WVGA(800x480) 时序参考值：
 * - 水平：Active=800, HSYNC=1,  HBP=46, HFP=210 => TotalW=1057
 * - 垂直：Active=480, VSYNC=1,  VBP=23, VFP=22  => TotalH=526
 * - 60Hz 像素时钟：PCLK ≈ 1057 * 526 * 60 ≈ 33.36 MHz
 *
 * 注意：不同屏幕差异很大，如仍出现偏移/抖动/闪屏，请以面板手册为准调整以下参数。
 */
#define LCD_HSYNC 1u
#define LCD_HBP 46u
#define LCD_HFP 210u
#define LCD_VSYNC 1u
#define LCD_VBP 23u
#define LCD_VFP 22u

/* ==========================
 * 像素时钟（PCLK）频率提示
 * ========================== */
/*
 * PCLK（像素时钟）需要通过 PLLSAI 给 LTDC 提供。
 * 不同面板/刷新率需要不同 PCLK（大约 = TotalW * TotalH * FPS）。
 *
 * 本示例在代码里给出一组“能编译、可调”的 PLLSAI 配置，但无法保证与你的屏匹配。
 * 如果出现花屏/不亮/闪烁，多半是 PCLK 或时序不对。
 */

/* ==========================
 * 颜色工具（RGB565）
 * ========================== */
#define LCD_RGB565(r5, g6, b5)                                                 \
  (uint16_t)((((uint16_t)(r5) & 0x1Fu) << 11) |                                \
             (((uint16_t)(g6) & 0x3Fu) << 5) |                                 \
             (((uint16_t)(b5) & 0x1Fu) << 0))

#define LCD_COLOR_BLACK_RGB565 LCD_RGB565(0, 0, 0)
#define LCD_COLOR_WHITE_RGB565 LCD_RGB565(31, 63, 31)
#define LCD_COLOR_RED_RGB565 LCD_RGB565(31, 0, 0)
#define LCD_COLOR_GREEN_RGB565 LCD_RGB565(0, 63, 0)
#define LCD_COLOR_BLUE_RGB565 LCD_RGB565(0, 0, 31)
