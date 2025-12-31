#include "ser_lvgl.h"

#include "FreeRTOS.h"
#include "task.h"

#include "dri_lcd_ltdc.h"
#include "main.h"

/*
 * 通过 __has_include 在“未引入 LVGL 源码”阶段保持工程可编译
 *
 * 你后续添加 LVGL 时，确保 include 路径里能找到：
 * - lvgl.h
 * - lv_conf.h（或在 lvgl.h 的配置路径中可找到）
 */
#if defined(__has_include)
#if __has_include("lvgl.h")
#define SER_LVGL_HAS_LIB 1
#else
#define SER_LVGL_HAS_LIB 0
#endif
#else
#define SER_LVGL_HAS_LIB 0
#endif

#if SER_LVGL_HAS_LIB
#include "lvgl.h"

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area,
                          uint8_t *px_map)
{
  (void)area;
  (void)px_map;

  /*
   * 当前先走“帧缓冲直写”模式：
   * - 使用 LV_DISPLAY_RENDER_MODE_DIRECT：framebuffer 直接作为 LVGL 的渲染目标
   * - flush 回调只需要告诉 LVGL “刷新完成”
   *
   * 后续你若需要提升性能，可改为：
   * - 双缓冲 + DMA2D（拷贝/混合加速）
   * - 或者 PARTIAL 模式做区域刷新
   */
  lv_display_flush_ready(disp);
}

static void lvgl_task(void *argument)
{
  (void)argument;

  lv_init();

  /* 创建并配置 display（LVGL v9 API） */
  lv_display_t *disp = lv_display_create(PIXELS_W, PIXELS_H);
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(disp, lvgl_flush_cb);

  /*
   * DIRECT 模式：framebuffer 直接作为 LVGL 绘制目标
   * - buf_size 以“字节”为单位
   */
  void *fb = dri_lcd_framebuffer();
  uint32_t fb_size = (uint32_t)PIXELS_W * (uint32_t)PIXELS_H * 2u;
  lv_display_set_buffers(disp, fb, NULL, fb_size, LV_DISPLAY_RENDER_MODE_DIRECT);

  /* 创建一个中文 label 作为移植验证 */
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "中文测试：你好，LVGL 9.4.0");
  lv_obj_center(label);

  for (;;)
  {
    /*
     * LVGL 轮询处理：
     * - 默认推荐 5~10ms 调一次
     * - 这里用 vTaskDelay 实现，后续你也可以换成更精确的定时器/notify 机制
     */
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void ser_lvgl_start(void)
{
  (void)xTaskCreate(lvgl_task, "lvgl", 2048, NULL, tskIDLE_PRIORITY + 2, NULL);
}

void ser_lvgl_tick_inc_isr(uint32_t ms)
{
  /*
   * SysTick_Handler 里调用该函数即可：
   * - 本工程 tick 频率为 1000Hz（1ms）
   * - ms 参数通常传 1
   */
  lv_tick_inc(ms);
}

#else /* SER_LVGL_HAS_LIB == 0 */

void ser_lvgl_start(void)
{
  /* LVGL 未集成时保持空实现，便于你分阶段移植 */
}

void ser_lvgl_tick_inc_isr(uint32_t ms)
{
  (void)ms;
}

#endif
