#include "boa_ultrasonic.h"

#include <stdbool.h>

static bool s_inited = false;

static void gpio_clk_enable(GPIO_TypeDef *port)
{
  if (port == GPIOA)
    __HAL_RCC_GPIOA_CLK_ENABLE();
  else if (port == GPIOB)
    __HAL_RCC_GPIOB_CLK_ENABLE();
  else if (port == GPIOC)
    __HAL_RCC_GPIOC_CLK_ENABLE();
  else if (port == GPIOD)
    __HAL_RCC_GPIOD_CLK_ENABLE();
  else if (port == GPIOE)
    __HAL_RCC_GPIOE_CLK_ENABLE();
  else if (port == GPIOF)
    __HAL_RCC_GPIOF_CLK_ENABLE();
  else if (port == GPIOG)
    __HAL_RCC_GPIOG_CLK_ENABLE();
  else if (port == GPIOH)
    __HAL_RCC_GPIOH_CLK_ENABLE();
  else if (port == GPIOI)
    __HAL_RCC_GPIOI_CLK_ENABLE();
}

void boa_ultrasonic_gpio_init(void)
{
  if (s_inited)
  {
    return;
  }

  gpio_clk_enable(BOA_US_TRIG_PORT);
  gpio_clk_enable(BOA_US_ECHO_PORT);

  GPIO_InitTypeDef gpio = {0};

  /* TRIG：推挽输出 */
  gpio.Pin = BOA_US_TRIG_PIN;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(BOA_US_TRIG_PORT, &gpio);
  HAL_GPIO_WritePin(BOA_US_TRIG_PORT, BOA_US_TRIG_PIN, GPIO_PIN_RESET);

  /*
   * ECHO：输入 + 边沿中断（用于避免任务调度/中断导致的“脉冲丢失”）
   *
   * 注意：当前工程在 mcu/core/stm32f4xx_it.c 里实现了 EXTI2_IRQHandler，
   * 因此默认 ECHO_PIN=GPIO_PIN_2 时可直接工作；若你改为其他 pin，请同步改 IRQ。
   */
  gpio.Pin = BOA_US_ECHO_PIN;
  gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
  gpio.Pull = BOA_US_ECHO_PULL;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(BOA_US_ECHO_PORT, &gpio);

  if (BOA_US_ECHO_PIN == GPIO_PIN_2)
  {
    HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  }

  s_inited = true;
}

void boa_ultrasonic_trig_write(bool high)
{
  boa_ultrasonic_gpio_init();
  HAL_GPIO_WritePin(BOA_US_TRIG_PORT, BOA_US_TRIG_PIN,
                    high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool boa_ultrasonic_echo_read(void)
{
  boa_ultrasonic_gpio_init();
  return (HAL_GPIO_ReadPin(BOA_US_ECHO_PORT, BOA_US_ECHO_PIN) == GPIO_PIN_SET);
}
