#include "boa_lcd_backlight.h"

#include "stm32f4xx_hal.h"

/*
 * 背光引脚：
 * - PD7: LCD_BL
 */
#define BOA_LCD_BL_GPIO_PORT GPIOD
#define BOA_LCD_BL_GPIO_PIN GPIO_PIN_7

static bool s_inited = false;

void boa_lcd_backlight_init(void)
{
  if (s_inited)
  {
    return;
  }

  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_InitTypeDef gpio = {0};
  gpio.Pin = BOA_LCD_BL_GPIO_PIN;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BOA_LCD_BL_GPIO_PORT, &gpio);

  s_inited = true;
}

void boa_lcd_backlight_set(bool on)
{
  boa_lcd_backlight_init();

  HAL_GPIO_WritePin(BOA_LCD_BL_GPIO_PORT, BOA_LCD_BL_GPIO_PIN,
                    on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
