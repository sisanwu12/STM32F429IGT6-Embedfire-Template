#include "ser_lvgl.h"

#include "FreeRTOS.h"
#include "task.h"

#include "dev_lcd.h"
#include "dev_touch.h"
#include "ser_ultrasonic.h"

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

typedef struct
{
  lv_obj_t *bar_bg;
  lv_obj_t *bar_fill;

  /* 超声波距离显示 */
  lv_obj_t *dist_label;

  /* 动画参数缓存 */
  uint32_t bar_pulse_ms;
} ui_boot_t;

static void ui_ultrasonic_timer_cb(lv_timer_t *t);

static void ui_set_width(void *obj, int32_t v)
{
  lv_obj_set_width((lv_obj_t *)obj, v);
}

static void ui_set_bg_opa(void *obj, int32_t v)
{
  if (v < 0)
    v = 0;
  if (v > 255)
    v = 255;
  lv_obj_set_style_bg_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}

static uint32_t ui_clamp_u32(uint32_t v, uint32_t lo, uint32_t hi)
{
  if (v < lo)
    return lo;
  if (v > hi)
    return hi;
  return v;
}

static lv_color_t ui_color_lerp(lv_color_t a, lv_color_t b, uint32_t t_0_1000)
{
  uint32_t t = ui_clamp_u32(t_0_1000, 0u, 1000u);
  uint32_t ia = 1000u - t;
  uint32_t r = (uint32_t)a.red * ia + (uint32_t)b.red * t;
  uint32_t g = (uint32_t)a.green * ia + (uint32_t)b.green * t;
  uint32_t bl = (uint32_t)a.blue * ia + (uint32_t)b.blue * t;
  return lv_color_make((uint8_t)(r / 1000u), (uint8_t)(g / 1000u),
                       (uint8_t)(bl / 1000u));
}

static void ui_bar_set_color(ui_boot_t *ui, uint32_t mm, bool valid)
{
  if (ui == NULL || ui->bar_fill == NULL)
  {
    return;
  }

  if (!valid)
  {
    lv_obj_set_style_bg_color(ui->bar_fill, lv_color_hex(0x56607A), 0);
    lv_obj_set_style_bg_grad_color(ui->bar_fill, lv_color_hex(0x7A86A3), 0);
    return;
  }

  /* 0mm(近)->红；4000mm(远)->蓝 */
  const uint32_t max_mm = 4000u;
  uint32_t t = ui_clamp_u32(mm, 0u, max_mm) * 1000u / max_mm;
  lv_color_t near_c = lv_color_hex(0xFF4D4D);
  lv_color_t far_c = lv_color_hex(0x3D7BFF);
  lv_color_t c = ui_color_lerp(near_c, far_c, t);

  lv_obj_set_style_bg_color(ui->bar_fill, c, 0);
  /* 渐变色稍微偏亮，增强“流光”感 */
  lv_color_t c2 = ui_color_lerp(c, lv_color_hex(0xFFFFFF), 220u);
  lv_obj_set_style_bg_grad_color(ui->bar_fill, c2, 0);
}

static void ui_bar_set_width(ui_boot_t *ui, uint32_t mm, bool valid)
{
  if (ui == NULL || ui->bar_bg == NULL || ui->bar_fill == NULL)
  {
    return;
  }

  int32_t bg_w = lv_obj_get_width(ui->bar_bg);
  if (bg_w <= 1)
  {
    return;
  }

  uint32_t target_w = 1u;
  if (valid)
  {
    /*
     * 更直观的“接近感”：
     * - 越近条越满
     * - 越远条越空
     */
    const uint32_t max_mm = 4000u; /* 400cm */
    uint32_t mm_c = ui_clamp_u32(mm, 0u, max_mm);
    uint32_t danger = max_mm - mm_c;
    target_w = 1u + (danger * (uint32_t)(bg_w - 1)) / max_mm;
  }

  uint32_t cur_w = (uint32_t)lv_obj_get_width(ui->bar_fill);
  if (cur_w == target_w)
  {
    return;
  }

  lv_anim_delete(ui->bar_fill, ui_set_width);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, ui->bar_fill);
  lv_anim_set_exec_cb(&a, ui_set_width);
  lv_anim_set_duration(&a, 160);
  lv_anim_set_values(&a, (int32_t)cur_w, (int32_t)target_w);
  lv_anim_start(&a);
}

static void ui_bar_set_pulse(ui_boot_t *ui, uint32_t mm, bool valid)
{
  if (ui == NULL || ui->bar_fill == NULL)
  {
    return;
  }

  uint32_t pulse_ms = 1200u;
  if (!valid)
  {
    pulse_ms = 1200u;
  }
  else
  {
    /* 越近闪烁越快：200ms~1200ms */
    const uint32_t max_mm = 4000u;
    uint32_t mm_c = ui_clamp_u32(mm, 0u, max_mm);
    pulse_ms = 200u + (mm_c * 1000u) / max_mm;
  }

  /* 变化不大就不重建动画，避免频繁重启 */
  if (ui->bar_pulse_ms != 0u)
  {
    uint32_t diff = (ui->bar_pulse_ms > pulse_ms) ? (ui->bar_pulse_ms - pulse_ms)
                                                  : (pulse_ms - ui->bar_pulse_ms);
    if (diff < 80u)
    {
      return;
    }
  }
  ui->bar_pulse_ms = pulse_ms;

  lv_anim_delete(ui->bar_fill, ui_set_bg_opa);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, ui->bar_fill);
  lv_anim_set_exec_cb(&a, ui_set_bg_opa);
  lv_anim_set_duration(&a, (uint32_t)pulse_ms);
  lv_anim_set_reverse_duration(&a, (uint32_t)pulse_ms);
  lv_anim_set_values(&a, 110, 255);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a);
}

static void ui_ultrasonic_timer_cb(lv_timer_t *t)
{
  ui_boot_t *ui = (ui_boot_t *)lv_timer_get_user_data(t);
  if (ui == NULL || ui->dist_label == NULL)
  {
    return;
  }

  uint32_t mm = 0;
  if (ser_ultrasonic_get_latest_mm(&mm))
  {
    uint32_t cm_int = mm / 10u;
    uint32_t cm_frac = mm % 10u;
    lv_label_set_text_fmt(ui->dist_label, "Dist: %lu.%01lu cm",
                          (unsigned long)cm_int, (unsigned long)cm_frac);

    ui_bar_set_width(ui, mm, true);
    ui_bar_set_color(ui, mm, true);
    ui_bar_set_pulse(ui, mm, true);
  }
  else
  {
    lv_label_set_text(ui->dist_label, "Dist: -- cm");

    ui_bar_set_width(ui, 0u, false);
    ui_bar_set_color(ui, 0u, false);
    ui_bar_set_pulse(ui, 0u, false);
  }
}

static void lvgl_indev_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
  (void)indev;

  bool pressed = false;
  uint16_t x = 0, y = 0;
  (void)dev_touch_read(&pressed, &x, &y);

  data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
  /* 防止触摸坐标超出屏幕导致“点到了但控件收不到” */
  const uint16_t w = dev_lcd_width();
  const uint16_t h = dev_lcd_height();
  if (w == 0u || h == 0u)
  {
    data->state = LV_INDEV_STATE_RELEASED;
    data->point.x = 0;
    data->point.y = 0;
    return;
  }
  if (x >= w)
    x = (uint16_t)(w - 1u);
  if (y >= h)
    y = (uint16_t)(h - 1u);

  data->point.x = (int32_t)x;
  data->point.y = (int32_t)y;
}

static void ui_boot_screen_create(ui_boot_t *ui)
{
  /* ===== 背景 ===== */
  lv_obj_t *scr = lv_screen_active();
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x0B1020), 0);
  lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x121A33), 0);
  lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_VER, 0);

  /* ===== 中央卡片 ===== */
  int32_t card_w = (int32_t)dev_lcd_width() * 72 / 100;
  int32_t card_h = (int32_t)dev_lcd_height() * 58 / 100;
  if (card_w > 520)
    card_w = 520;
  if (card_h > 320)
    card_h = 320;

  lv_obj_t *card = lv_obj_create(scr);
  lv_obj_set_size(card, card_w, card_h);
  lv_obj_center(card);
  lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_radius(card, 22, 0);
  lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(card, lv_color_hex(0x16213E), 0);
  lv_obj_set_style_bg_grad_color(card, lv_color_hex(0x1E2B52), 0);
  lv_obj_set_style_bg_grad_dir(card, LV_GRAD_DIR_VER, 0);
  lv_obj_set_style_border_width(card, 1, 0);
  lv_obj_set_style_border_color(card, lv_color_hex(0x2B3A67), 0);
  lv_obj_set_style_shadow_width(card, 24, 0);
  lv_obj_set_style_shadow_opa(card, LV_OPA_30, 0);
  lv_obj_set_style_shadow_color(card, lv_color_hex(0x000000), 0);
  lv_obj_set_style_pad_all(card, 24, 0);
  lv_obj_set_style_pad_row(card, 10, 0);

  /* ===== Logo（简易圆形徽章） ===== */
  lv_obj_t *badge = lv_obj_create(card);
  lv_obj_set_size(badge, 64, 64);
  lv_obj_set_style_radius(badge, 32, 0);
  lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(badge, lv_color_hex(0x3D7BFF), 0);
  lv_obj_set_style_bg_grad_color(badge, lv_color_hex(0x67D7FF), 0);
  /* LVGL v9.4.0 不支持对角渐变方向，使用径向渐变来增强“徽章”质感 */
  lv_obj_set_style_bg_grad_dir(badge, LV_GRAD_DIR_RADIAL, 0);
  lv_obj_set_style_border_width(badge, 0, 0);
  lv_obj_set_style_shadow_width(badge, 18, 0);
  /* v9 中透明度常量以 5/10/20/30... 为主，这里选一个相近值 */
  lv_obj_set_style_shadow_opa(badge, LV_OPA_30, 0);
  lv_obj_set_style_shadow_color(badge, lv_color_hex(0x3D7BFF), 0);
  lv_obj_align(badge, LV_ALIGN_TOP_MID, 0, 0);

  /* ===== 标题 ===== */
  lv_obj_t *title = lv_label_create(card);
  /* 这里用到的汉字确保已在内置字体中包含（例如：歡/迎/使/用）。 */
  lv_label_set_text(title, "歡迎使用");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_letter_space(title, 2, 0);
  lv_obj_align_to(title, badge, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);

  /* ===== 副标题（包含中文与版本） ===== */
  lv_obj_t *sub = lv_label_create(card);
  /* 避免使用字体里可能不存在的符号（如 '·'），分隔符用 ASCII '|' 更稳妥。 */
  lv_label_set_text(sub, "STM32F429 | LVGL 9.4.0 | 中文可用");
  lv_obj_set_style_text_color(sub, lv_color_hex(0xB8C7FF), 0);
  lv_obj_set_style_text_opa(sub, LV_OPA_90, 0);
  lv_obj_set_style_text_letter_space(sub, 1, 0);
  lv_obj_align_to(sub, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  /* ===== 超声波距离显示 ===== */
  ui->dist_label = lv_label_create(card);
  lv_label_set_text(ui->dist_label, "Dist: -- cm");
  lv_obj_set_style_text_color(ui->dist_label, lv_color_hex(0xE6EEFF), 0);
  lv_obj_set_style_text_opa(ui->dist_label, LV_OPA_90, 0);
  lv_obj_set_style_text_letter_space(ui->dist_label, 1, 0);
  lv_obj_align_to(ui->dist_label, sub, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  /* ===== 进度条（纯 lv_obj 实现） ===== */
  ui->bar_bg = lv_obj_create(card);
  lv_obj_set_height(ui->bar_bg, 12);
  lv_obj_set_width(ui->bar_bg, card_w - 48);
  lv_obj_set_style_radius(ui->bar_bg, 6, 0);
  lv_obj_set_style_bg_opa(ui->bar_bg, LV_OPA_30, 0);
  lv_obj_set_style_bg_color(ui->bar_bg, lv_color_hex(0x0A1024), 0);
  lv_obj_set_style_border_width(ui->bar_bg, 0, 0);
  lv_obj_align(ui->bar_bg, LV_ALIGN_BOTTOM_MID, 0, 0);

  ui->bar_fill = lv_obj_create(ui->bar_bg);
  lv_obj_set_height(ui->bar_fill, 12);
  lv_obj_set_width(ui->bar_fill, 1);
  lv_obj_set_style_radius(ui->bar_fill, 6, 0);
  lv_obj_set_style_bg_opa(ui->bar_fill, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(ui->bar_fill, lv_color_hex(0x3D7BFF), 0);
  lv_obj_set_style_bg_grad_color(ui->bar_fill, lv_color_hex(0x67D7FF), 0);
  lv_obj_set_style_bg_grad_dir(ui->bar_fill, LV_GRAD_DIR_HOR, 0);
  lv_obj_set_style_border_width(ui->bar_fill, 0, 0);
  lv_obj_align(ui->bar_fill, LV_ALIGN_LEFT_MID, 0, 0);

  /* 初始：等数据时显示最小值 + 慢速呼吸 */
  ui->bar_pulse_ms = 0u;
  ui_bar_set_width(ui, 0u, false);
  ui_bar_set_color(ui, 0u, false);
  ui_bar_set_pulse(ui, 0u, false);
}

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
  lv_display_t *disp = lv_display_create(dev_lcd_width(), dev_lcd_height());
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(disp, lvgl_flush_cb);

  /*
   * DIRECT 模式：framebuffer 直接作为 LVGL 绘制目标
   * - buf_size 以“字节”为单位
   */
  void *fb = dev_lcd_framebuffer();
  uint32_t fb_size =
      (uint32_t)dev_lcd_width() * (uint32_t)dev_lcd_height() * 2u;
  lv_display_set_buffers(disp, fb, NULL, fb_size,
                         LV_DISPLAY_RENDER_MODE_DIRECT);

  /* 绑定触摸输入（电容屏） */
  (void)dev_touch_init();
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_display(indev, disp);
  lv_indev_set_read_cb(indev, lvgl_indev_read_cb);

  /* 启动界面（含中文字体验证） */
  ui_boot_t ui = {0};
  ui_boot_screen_create(&ui);
  (void)lv_timer_create(ui_ultrasonic_timer_cb, 200, &ui);

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
  (void)xTaskCreate(lvgl_task, "lvgl", 4096, NULL, tskIDLE_PRIORITY + 2, NULL);
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

void ser_lvgl_start(void) { /* LVGL 未集成时保持空实现，便于你分阶段移植 */ }

void ser_lvgl_tick_inc_isr(uint32_t ms) { (void)ms; }

#endif
