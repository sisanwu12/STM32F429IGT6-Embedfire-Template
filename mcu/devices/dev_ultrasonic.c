#include "dev_ultrasonic.h"

#include "boa_ultrasonic.h"
#include "dri_time_us.h"

static bool s_inited = false;

static volatile uint32_t s_isr_start_cycles = 0;
static volatile uint32_t s_isr_pulse_us = 0;
static volatile uint8_t s_isr_rise_seen = 0;
static volatile uint8_t s_isr_done = 0;

void dev_ultrasonic_echo_edge_isr(void)
{
  if (!s_inited)
  {
    return;
  }

  bool level = boa_ultrasonic_echo_read();

  if (level)
  {
    /* 上升沿：记录起点（只记第一次，避免抖动重复覆盖） */
    if (!s_isr_rise_seen)
    {
      s_isr_rise_seen = 1u;
      s_isr_start_cycles = dri_time_cycles_now();
    }
    return;
  }

  /* 下降沿：只有在已见过上升沿后才计算脉宽 */
  if (s_isr_rise_seen && !s_isr_done)
  {
    uint32_t pulse = dri_time_cycles_elapsed_us(s_isr_start_cycles);
    s_isr_pulse_us = pulse;
    s_isr_done = 1u;
  }
}

bool dev_ultrasonic_init(void)
{
  if (s_inited)
  {
    return true;
  }

  boa_ultrasonic_gpio_init();
  if (dri_time_us_init() != HAL_OK)
  {
    return false;
  }

  s_inited = true;
  return true;
}

static bool wait_echo_level(bool level, uint32_t timeout_us)
{
  uint32_t start = dri_time_cycles_now();
  while (dri_time_cycles_elapsed_us(start) < timeout_us)
  {
    if (boa_ultrasonic_echo_read() == level)
    {
      return true;
    }
  }
  return false;
}

static bool wait_isr_flag(volatile uint8_t *flag, uint32_t timeout_us)
{
  uint32_t start = dri_time_cycles_now();
  while (dri_time_cycles_elapsed_us(start) < timeout_us)
  {
    if (*flag)
    {
      return true;
    }
  }
  return false;
}

bool dev_ultrasonic_measure_mm(uint32_t *mm)
{
  if (mm == NULL)
  {
    return false;
  }

  if (!s_inited && !dev_ultrasonic_init())
  {
    return false;
  }

  /* 触发前，确保 TRIG 为低，且 ECHO 回到低电平 */
  boa_ultrasonic_trig_write(false);
  if (!wait_echo_level(false, 2000u))
  {
    /* ECHO 一直高：可能是接线/电平不匹配/上一帧没结束 */
    return false;
  }

  /* TRIG > 10us（手册建议 50us 左右），这里取 60us */
  s_isr_pulse_us = 0;
  s_isr_rise_seen = 0u;
  s_isr_done = 0u;
  boa_ultrasonic_trig_write(true);
  dri_time_delay_us(60u);
  boa_ultrasonic_trig_write(false);

  /* 等待回波开始（上升沿） */
  /*
   * 大部分模块 ECHO 会很快拉高；但为了兼容“直到接收到回波才拉高”的实现，
   * 这里把等待上升沿的超时放宽到 40ms。
   */
  if (!wait_isr_flag(&s_isr_rise_seen, 40000u))
  {
    return false;
  }

  /* 等待回波结束（下降沿）；5.6m 往返约 33ms，这里给 40ms */
  if (!wait_isr_flag(&s_isr_done, 40000u))
  {
    return false;
  }

  uint32_t pulse_us = s_isr_pulse_us;
  if (pulse_us == 0u)
  {
    return false;
  }

  /*
   * 常用换算：
   * - 距离(cm) ≈ pulse_us / 58
   * - 距离(mm) ≈ pulse_us * 10 / 58
   */
  uint32_t mm_v = (pulse_us * 10u + 29u) / 58u;
  *mm = mm_v;
  return true;
}
