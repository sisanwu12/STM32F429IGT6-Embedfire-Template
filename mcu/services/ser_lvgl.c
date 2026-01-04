#include "ser_lvgl.h"

#include "FreeRTOS.h"
#include "task.h"

#include "dev_lcd.h"
#include "dev_touch.h"

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

  /* 计数按钮相关 */
  lv_obj_t *count_label;
  uint32_t count;
} ui_boot_t;

static void ui_count_update(ui_boot_t *ui)
{
  /* 只显示数字，避免中文缺字影响演示 */
  lv_label_set_text_fmt(ui->count_label, "%lu", (unsigned long)ui->count);
}

static void ui_set_width(void *obj, int32_t v)
{
  lv_obj_set_width((lv_obj_t *)obj, v);
}

static void ui_count_btn_event_cb(lv_event_t *e)
{
  ui_boot_t *ui = (ui_boot_t *)lv_event_get_user_data(e);
  if (ui == NULL)
  {
    return;
  }

  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    ui->count++;
    ui_count_update(ui);
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

  /* ===== 计数显示（数字） ===== */
  ui->count = 0;
  ui->count_label = lv_label_create(card);
  lv_label_set_text(ui->count_label, "0");
  lv_obj_set_style_text_color(ui->count_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_opa(ui->count_label, LV_OPA_90, 0);
  lv_obj_set_style_text_letter_space(ui->count_label, 1, 0);

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

  /* ===== “+1” 按钮（用 lv_obj 模拟，避免额外搬运按钮控件） ===== */
  lv_obj_t *btn = lv_obj_create(card);
  lv_obj_set_size(btn, 160, 48);
  lv_obj_set_style_radius(btn, 14, 0);
  lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(btn, lv_color_hex(0x2B3A67), 0);
  lv_obj_set_style_border_width(btn, 1, 0);
  lv_obj_set_style_border_color(btn, lv_color_hex(0x3D7BFF), 0);
  lv_obj_set_style_shadow_width(btn, 14, 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_30, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_hex(0x000000), 0);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(btn, ui_count_btn_event_cb, LV_EVENT_CLICKED, ui);
  lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -28);

  lv_obj_t *btn_txt = lv_label_create(btn);
  lv_label_set_text(btn_txt, "Tap +1");
  lv_obj_set_style_text_color(btn_txt, lv_color_hex(0xFFFFFF), 0);
  lv_obj_center(btn_txt);

  /* 把数字放在按钮上方，便于观察 */
  lv_obj_align_to(ui->count_label, btn, LV_ALIGN_OUT_TOP_MID, 0, -10);
  ui_count_update(ui);

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

  /* ===== 动画：进度条来回流动 ===== */
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, ui->bar_fill);
  lv_anim_set_exec_cb(&a, ui_set_width);
  lv_anim_set_duration(&a, 1200);
  lv_anim_set_values(&a, 1, lv_obj_get_width(ui->bar_bg));
  lv_anim_set_reverse_duration(&a, 800);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a);
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
