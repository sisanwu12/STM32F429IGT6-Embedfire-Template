#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * devices/ 层：电容触摸屏（外部器件）抽象接口
 *
 * 当前实现：GT911（常见 7 寸 800x480 电容屏）
 * - 若你的触控 IC 不是 GT911，文件仍可编译，但触摸读数可能不正确
 */

bool dev_touch_init(void);

/*
 * 读取单点触摸（本工程用于 LVGL 指针输入）
 * - pressed: true 表示按下
 * - x/y: 触摸点坐标（像素坐标系，原点左上角）
 */
bool dev_touch_read(bool *pressed, uint16_t *x, uint16_t *y);

/* 调试信息：用于现场定位 I2C/地址/读写失败原因 */
uint16_t dev_touch_debug_addr_7bit(void);
uint8_t dev_touch_debug_last_err(void);
uint8_t dev_touch_debug_i2c_ready_mask(void);
uint8_t dev_touch_debug_bus_id(void);
uint8_t dev_touch_debug_last_status(void);
uint8_t dev_touch_debug_int_active(void);
uint8_t dev_touch_debug_int_level_high(void);
const char *dev_touch_debug_product_id(void);
uint8_t dev_touch_debug_reg_order(void);
uint8_t dev_touch_debug_status_msb(void);
uint8_t dev_touch_debug_status_lsb(void);
uint16_t dev_touch_debug_cfg_x(void);
uint16_t dev_touch_debug_cfg_y(void);
uint16_t dev_touch_debug_cfg_x_msb(void);
uint16_t dev_touch_debug_cfg_y_msb(void);
uint16_t dev_touch_debug_cfg_x_lsb(void);
uint16_t dev_touch_debug_cfg_y_lsb(void);
const char *dev_touch_debug_pid_hex_msb(void);
const char *dev_touch_debug_pid_hex_lsb(void);
const char *dev_touch_debug_cfg_hex_msb(void);
const char *dev_touch_debug_cfg_hex_lsb(void);
const char *dev_touch_debug_r40_hex(void);
const char *dev_touch_debug_r4e_hex(void);
const char *dev_touch_debug_8140_hex(void);
const char *dev_touch_debug_8150_hex(void);
uint16_t dev_touch_debug_diff_addr(void);
const char *dev_touch_debug_diff_hex(void);
const char *dev_touch_debug_i2c_scan(void);
uint8_t dev_touch_debug_rst_low_ready_mask(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif
