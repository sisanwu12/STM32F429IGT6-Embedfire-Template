#include "boa_touch.h"

#include "stm32f4xx_hal.h"

/* INT: PD13 */
#define BOA_CTP_INT_PORT GPIOD
#define BOA_CTP_INT_PIN GPIO_PIN_13

/* RST: PI8 */
#define BOA_CTP_RST_PORT GPIOI
#define BOA_CTP_RST_PIN GPIO_PIN_8

static bool s_inited = false;

void boa_touch_gpio_init(void)
{
  if (s_inited)
  {
    return;
  }

  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();

  /* RST 默认输出 */
  GPIO_InitTypeDef gpio = {0};
  gpio.Pin = BOA_CTP_RST_PIN;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BOA_CTP_RST_PORT, &gpio);

  /* INT 默认输入（上拉可按硬件调整） */
  gpio.Pin = BOA_CTP_INT_PIN;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BOA_CTP_INT_PORT, &gpio);

  s_inited = true;
}

static void rst_write(bool high)
{
  HAL_GPIO_WritePin(BOA_CTP_RST_PORT, BOA_CTP_RST_PIN,
                    high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void boa_touch_reset_pulse(void)
{
  boa_touch_gpio_init();

  /* 常见 CTP 复位：拉低一段时间再拉高 */
  rst_write(false);
  HAL_Delay(10);
  rst_write(true);
  HAL_Delay(50);
}

bool boa_touch_int_is_active(void)
{
  boa_touch_gpio_init();
  return (HAL_GPIO_ReadPin(BOA_CTP_INT_PORT, BOA_CTP_INT_PIN) == GPIO_PIN_RESET);
}

void boa_touch_int_set_output(bool output)
{
  boa_touch_gpio_init();

  GPIO_InitTypeDef gpio = {0};
  gpio.Pin = BOA_CTP_INT_PIN;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  gpio.Mode = output ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
  HAL_GPIO_Init(BOA_CTP_INT_PORT, &gpio);
}

void boa_touch_int_write(bool high)
{
  boa_touch_int_set_output(true);
  HAL_GPIO_WritePin(BOA_CTP_INT_PORT, BOA_CTP_INT_PIN,
                    high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

